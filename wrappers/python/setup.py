import os
import platform
import subprocess
import sys

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


# Adapted from here: https://github.com/pybind/cmake_example/blob/master/setup.py
class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            subprocess.check_output(['cmake', '--version'])
        except OSError:
            sys.exit("CMake must be installed to build the python wrapper")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j2']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        try:
            subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        except subprocess.CalledProcessError:
            sys.exit("Error running cmake configure step")
        try:
            subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)
        except subprocess.CalledProcessError:
            sys.exit("Error running cmake build step")


with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()


setup(
    name='zxing-cpp',
    version='0.0.1',
    # use_scm_version={
    #     "root": "../..",
    #     "version_scheme": "guess-next-dev",
    #     "local_scheme": "no-local-version",
    #     "tag_regex": "v?([0-9]+.[0-9]+.[0-9]+)",
    # },
    description='Python bindings for the zxing-cpp barcode library',
    long_description=long_description,
    long_description_content_type="text/markdown",
    author='Timothy Rae',
    author_email='timothy.rae@ankidroid.org',
    url='https://github.com/nu-book/zxing-cpp',
    license='Apache License 2.0',
    keywords=['barcode'],
    classifiers=[
        "Development Status :: 4 - Beta",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
        "Topic :: Multimedia :: Graphics",
    ],
    python_requires=">=3.6",
    ext_modules=[CMakeExtension('zxing')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
)
