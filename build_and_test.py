#!/usr/bin/env python3

from os.path import dirname, splitext, basename
import glob
import os
import sys


# Compile docopt usage to C
# docopt_path = f'{dirname(__file__)}/src/docopt/docopt.c/docopt_c.py'
# for filename in glob.glob('src/docopt/*.docopt'):
#     name, _ = splitext(basename(filename))
#     os.system(f'{sys.executable} {docopt_path} {filename} -o src/{name}_args.c')

# Clear build directory before building
if 'rebuild' in sys.argv:
    if sys.platform == 'win32':
        os.system('del /F /Q build')
    else:
        os.system('rm -rf build')

# Create build directory
if not os.path.exists('build'):
    os.system('mkdir build')
os.chdir('build')

# Run cmake and whatever os-specific build tool.
os.system('cmake ..')
if sys.platform == 'win32':
    os.system('cmake --build . --config Release')
    os.chdir('..')
    # os.system('.\\build\\Release\\test_runner.exe')
else:
    os.system('make')
    os.chdir('..')
    # os.system('./build/i106stat')
