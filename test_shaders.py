#!/usr/bin/env python3

import sys
import os
import subprocess
import tempfile
import re
import itertools
import hashlib
import shutil

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

def cross_compile(shader):
    spirv_f, spirv_path = tempfile.mkstemp()
    glsl_f, glsl_path = tempfile.mkstemp(suffix = os.path.basename(shader))
    os.close(spirv_f)
    os.close(glsl_f)

    subprocess.check_call(['glslangValidator', '-G', '-o', spirv_path, shader])
    subprocess.check_call(['./spir2cross', '--output', glsl_path, spirv_path])
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

def regression_check(shader, glsl):
    reference = os.path.join('./reference', shader)
    if os.path.exists(reference):
        if md5_for_file(glsl) != md5_for_file(reference):
            print('Generated GLSL has changed for {}!'.format(reference))
            if os.path.exists(reference):
                os.remove(reference)
            make_reference_dir(reference)
            shutil.move(glsl, reference)
        else:
            os.remove(glsl)
    else:
        print('Found new shader {}. Placing GLSL in {}'.format(shader, reference))
        make_reference_dir(reference)
        shutil.move(glsl, reference)

def test_shader(stats, shader):
    print('Testing shader:', shader)
    pristine_stats = get_shader_stats(shader)
    spirv, glsl = cross_compile(shader)
    cross_stats = get_shader_stats(glsl)

    regression_check(shader, glsl)
    os.remove(spirv)

    a = []
    a.append(shader)
    for i in pristine_stats:
        a.append(str(i))
    for i in cross_stats:
        a.append(str(i))
    print(','.join(a), file = stats)

def test_shaders(shader_dir):
    with open('stats.csv', 'w') as stats:
        print('Shader,OrigRegs,OrigUniRegs,OrigALUShort,OrigLSShort,OrigTEXShort,OrigALULong,OrigLSLong,OrigTEXLong,CrossRegs,CrossUniRegs,CrossALUShort,CrossLSShort,CrossTEXShort,CrossALULong,CrossLSLong,CrossTEXLong', file = stats)
        for f in os.walk(os.path.join(shader_dir)):
            for i in f[2]:
                shader = os.path.join(f[0], i)
                test_shader(stats, shader)

if __name__ == '__main__':
    test_shaders(sys.argv[1])
    print('Stats in stats.csv!')
