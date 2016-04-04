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

def validate_shader(shader):
    subprocess.check_call(['glslangValidator', shader])

def cross_compile(shader):
    spirv_f, spirv_path = tempfile.mkstemp()
    glsl_f, glsl_path = tempfile.mkstemp(suffix = os.path.basename(shader))
    os.close(spirv_f)
    os.close(glsl_f)

    subprocess.check_call(['glslangValidator', '-G', '-o', spirv_path, shader])
    subprocess.check_call(['./spirv-cross', '--output', glsl_path, spirv_path])
    validate_shader(glsl_path)
    return (spirv_path, glsl_path)

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

def regression_check(shader, glsl, update, keep):
    reference = os.path.join('./reference', shader)
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
                # Otherwise, fail the test. Keep the shader file around so we can inspect.
                if not keep:
                    os.remove(glsl)
                sys.exit(1)
        else:
            os.remove(glsl)
    else:
        print('Found new shader {}. Placing GLSL in {}'.format(shader, reference))
        make_reference_dir(reference)
        shutil.move(glsl, reference)

def test_shader(stats, shader, update, keep):
    print('Testing shader:', shader)
    spirv, glsl = cross_compile(shader)

    if stats:
        cross_stats = get_shader_stats(glsl)

    regression_check(shader, glsl, update, keep)
    os.remove(spirv)

    if stats:
        pristine_stats = get_shader_stats(shader)

        a = []
        a.append(shader)
        for i in pristine_stats:
            a.append(str(i))
        for i in cross_stats:
            a.append(str(i))
        print(','.join(a), file = stats)

def test_shaders(shader_dir, update, malisc, keep):
    if malisc:
        with open('stats.csv', 'w') as stats:
            print('Shader,OrigRegs,OrigUniRegs,OrigALUShort,OrigLSShort,OrigTEXShort,OrigALULong,OrigLSLong,OrigTEXLong,CrossRegs,CrossUniRegs,CrossALUShort,CrossLSShort,CrossTEXShort,CrossALULong,CrossLSLong,CrossTEXLong', file = stats)
            for f in os.walk(os.path.join(shader_dir)):
                for i in f[2]:
                    shader = os.path.join(f[0], i)
                    test_shader(stats, shader, update, keep)
    else:
        for f in os.walk(os.path.join(shader_dir)):
            for i in f[2]:
                shader = os.path.join(f[0], i)
                test_shader(None, shader, update, keep)

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
    args = parser.parse_args()

    if not args.folder:
        sys.stderr.write('Need shader folder.\n')
        sys.exit(1)

    test_shaders(args.folder, args.update, args.malisc, args.keep)
    if args.malisc:
        print('Stats in stats.csv!')
    print('Tests completed!')

if __name__ == '__main__':
    main()
