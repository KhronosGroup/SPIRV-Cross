#!/usr/bin/env python3

import sys
import os
import subprocess
import tempfile
import re
import itertools
import hashlib
import shutil
import argparse

def parse_stats(stats):
    m = re.search('([0-9]+) work registers', stats)
    registers = int(m.group(1)) if m else 0

    m = re.search('([0-9]+) uniform registers', stats)
    uniform_regs = int(m.group(1)) if m else 0

    m_list = re.findall('(-?[0-9]+)\s+(-?[0-9]+)\s+(-?[0-9]+)', stats)
    alu_short = float(m_list[1][0]) if m_list else 0
    ls_short = float(m_list[1][1]) if m_list else 0
    tex_short = float(m_list[1][2]) if m_list else 0
    alu_long = float(m_list[2][0]) if m_list else 0
    ls_long = float(m_list[2][1]) if m_list else 0
    tex_long = float(m_list[2][2]) if m_list else 0

    return (registers, uniform_regs, alu_short, ls_short, tex_short, alu_long, ls_long, tex_long)

def get_shader_type(shader):
    _, ext = os.path.splitext(shader)
    if ext == '.vert':
        return '--vertex'
    elif ext == '.frag':
        return '--fragment'
    elif ext == '.comp':
        return '--compute'
    elif ext == '.tesc':
        return '--tessellation_control'
    elif ext == '.tese':
        return '--tessellation_evaluation'
    elif ext == '.geom':
        return '--geometry'
    else:
        return ''

def get_shader_stats(shader):
    f, path = tempfile.mkstemp()

    os.close(f)
    p = subprocess.Popen(['malisc', get_shader_type(shader), '--core', 'Mali-T760', '-V', shader], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    stdout, stderr = p.communicate()
    os.remove(path)

    if p.returncode != 0:
        print(stderr.decode('utf-8'))
        raise OSError('malisc failed')
    p.wait()

    returned = stdout.decode('utf-8')
    return parse_stats(returned)

def validate_shader(shader, vulkan):
    if vulkan:
        subprocess.check_call(['glslangValidator', '-V', shader])
    else:
        subprocess.check_call(['glslangValidator', shader])

def cross_compile_msl(shader):
    spirv_f, spirv_path = tempfile.mkstemp()
    msl_f, msl_path = tempfile.mkstemp(suffix = os.path.basename(shader))
    os.close(spirv_f)
    os.close(msl_f)
    subprocess.check_call(['glslangValidator', '-V', '-o', spirv_path, shader])
    spirv_cross_path = './spirv-cross'
    subprocess.check_call([spirv_cross_path, '--entry', 'main', '--output', msl_path, spirv_path, '--metal'])
    subprocess.check_call(['spirv-val', spirv_path])

    # TODO: Add optional validation of the MSL output.
    return (spirv_path, msl_path)

def cross_compile(shader, vulkan, spirv, invalid_spirv, eliminate, is_legacy, flatten_ubo):
    spirv_f, spirv_path = tempfile.mkstemp()
    glsl_f, glsl_path = tempfile.mkstemp(suffix = os.path.basename(shader))
    os.close(spirv_f)
    os.close(glsl_f)

    if vulkan or spirv:
        vulkan_glsl_f, vulkan_glsl_path = tempfile.mkstemp(suffix = os.path.basename(shader))
        os.close(vulkan_glsl_f)

    if spirv:
        subprocess.check_call(['spirv-as', '-o', spirv_path, shader])
    else:
        subprocess.check_call(['glslangValidator', '-V', '-o', spirv_path, shader])

    if not invalid_spirv:
        subprocess.check_call(['spirv-val', spirv_path])

    extra_args = []
    if eliminate:
        extra_args += ['--remove-unused-variables']
    if is_legacy:
        extra_args += ['--version', '100', '--es']
    if flatten_ubo:
        extra_args += ['--flatten-ubo']

    spirv_cross_path = './spirv-cross'
    subprocess.check_call([spirv_cross_path, '--entry', 'main', '--output', glsl_path, spirv_path] + extra_args)

    # A shader might not be possible to make valid GLSL from, skip validation for this case.
    if (not ('nocompat' in glsl_path)) and (not spirv):
        validate_shader(glsl_path, False)

    if vulkan or spirv:
        subprocess.check_call([spirv_cross_path, '--entry', 'main', '--vulkan-semantics', '--output', vulkan_glsl_path, spirv_path] + extra_args)
        validate_shader(vulkan_glsl_path, vulkan)

    return (spirv_path, glsl_path, vulkan_glsl_path if vulkan else None)

def md5_for_file(path):
    md5 = hashlib.md5()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(8192), b''):
            md5.update(chunk)
    return md5.digest()

def make_reference_dir(path):
    base = os.path.dirname(path)
    if not os.path.exists(base):
        os.makedirs(base)

def reference_path(directory, relpath):
    split_paths = os.path.split(directory)
    reference_dir = os.path.join(split_paths[0], 'reference/')
    reference_dir = os.path.join(reference_dir, split_paths[1])
    return os.path.join(reference_dir, relpath)

def regression_check(shader, glsl, update, keep):
    reference = reference_path(shader[0], shader[1])
    joined_path = os.path.join(shader[0], shader[1])
    print('Reference shader path:', reference)

    if os.path.exists(reference):
        if md5_for_file(glsl) != md5_for_file(reference):
            if update:
                print('Generated GLSL has changed for {}!'.format(reference))
                # If we expect changes, update the reference file.
                if os.path.exists(reference):
                    os.remove(reference)
                make_reference_dir(reference)
                shutil.move(glsl, reference)
            else:
                print('Generated GLSL in {} does not match reference {}!'.format(glsl, reference))
                with open(glsl, 'r') as f:
                    print('')
                    print('Generated:')
                    print('======================')
                    print(f.read())
                    print('======================')
                    print('')

                # Otherwise, fail the test. Keep the shader file around so we can inspect.
                if not keep:
                    os.remove(glsl)
                sys.exit(1)
        else:
            os.remove(glsl)
    else:
        print('Found new shader {}. Placing GLSL in {}'.format(joined_path, reference))
        make_reference_dir(reference)
        shutil.move(glsl, reference)

def shader_is_vulkan(shader):
    return '.vk.' in shader

def shader_is_desktop(shader):
    return '.desktop.' in shader

def shader_is_eliminate_dead_variables(shader):
    return '.noeliminate.' not in shader

def shader_is_spirv(shader):
    return '.asm.' in shader

def shader_is_invalid_spirv(shader):
    return '.invalid.' in shader

def shader_is_legacy(shader):
    return '.legacy.' in shader

def shader_is_flatten_ubo(shader):
    return '.flatten.' in shader

def test_shader(stats, shader, update, keep):
    joined_path = os.path.join(shader[0], shader[1])
    vulkan = shader_is_vulkan(shader[1])
    desktop = shader_is_desktop(shader[1])
    eliminate = shader_is_eliminate_dead_variables(shader[1])
    is_spirv = shader_is_spirv(shader[1])
    invalid_spirv = shader_is_invalid_spirv(shader[1])
    is_legacy = shader_is_legacy(shader[1])
    flatten_ubo = shader_is_flatten_ubo(shader[1])

    print('Testing shader:', joined_path)
    spirv, glsl, vulkan_glsl = cross_compile(joined_path, vulkan, is_spirv, invalid_spirv, eliminate, is_legacy, flatten_ubo)

    # Only test GLSL stats if we have a shader following GL semantics.
    if stats and (not vulkan) and (not is_spirv) and (not desktop):
        cross_stats = get_shader_stats(glsl)

    regression_check(shader, glsl, update, keep)
    if vulkan_glsl:
        regression_check((shader[0], shader[1] + '.vk'), vulkan_glsl, update, keep)
    os.remove(spirv)

    if stats and (not vulkan) and (not is_spirv) and (not desktop):
        pristine_stats = get_shader_stats(joined_path)

        a = []
        a.append(shader[1])
        for i in pristine_stats:
            a.append(str(i))
        for i in cross_stats:
            a.append(str(i))
        print(','.join(a), file = stats)

def test_shader_msl(stats, shader, update, keep):
    joined_path = os.path.join(shader[0], shader[1])
    print('Testing MSL shader:', joined_path)
    spirv, msl = cross_compile_msl(joined_path)
    regression_check(shader, msl, update, keep)
    os.remove(spirv)

def test_shaders_helper(stats, shader_dir, update, malisc, keep, backend):
    for root, dirs, files in os.walk(os.path.join(shader_dir)):
        for i in files:
            path = os.path.join(root, i)
            relpath = os.path.relpath(path, shader_dir)
            if backend == 'metal':
                test_shader_msl(stats, (shader_dir, relpath), update, keep)
            else:
                test_shader(stats, (shader_dir, relpath), update, keep)

def test_shaders(shader_dir, update, malisc, keep, backend):
    if malisc:
        with open('stats.csv', 'w') as stats:
            print('Shader,OrigRegs,OrigUniRegs,OrigALUShort,OrigLSShort,OrigTEXShort,OrigALULong,OrigLSLong,OrigTEXLong,CrossRegs,CrossUniRegs,CrossALUShort,CrossLSShort,CrossTEXShort,CrossALULong,CrossLSLong,CrossTEXLong', file = stats)
            test_shaders_helper(stats, shader_dir, update, malisc, keep, backend)
    else:
        test_shaders_helper(None, shader_dir, update, malisc, keep, backend)

def main():
    parser = argparse.ArgumentParser(description = 'Script for regression testing.')
    parser.add_argument('folder',
            help = 'Folder containing shader files to test.')
    parser.add_argument('--update',
            action = 'store_true',
            help = 'Updates reference files if there is a mismatch. Use when legitimate changes in output is found.')
    parser.add_argument('--keep',
            action = 'store_true',
            help = 'Leave failed GLSL shaders on disk if they fail regression. Useful for debugging.')
    parser.add_argument('--malisc',
            action = 'store_true',
            help = 'Use malisc offline compiler to determine static cycle counts before and after spirv-cross.')
    parser.add_argument('--metal',
            action = 'store_true',
            help = 'Test Metal backend.')
    args = parser.parse_args()

    if not args.folder:
        sys.stderr.write('Need shader folder.\n')
        sys.exit(1)

    test_shaders(args.folder, args.update, args.malisc, args.keep, 'metal' if args.metal else 'glsl')
    if args.malisc:
        print('Stats in stats.csv!')
    print('Tests completed!')

if __name__ == '__main__':
    main()
