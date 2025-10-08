import os
import sys
import platform
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
	def __init__(self, name, sourcedir=''):
		super().__init__(name, sources=[])
		self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
	def build_extension(self, ext):
		extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
		cfg = 'Debug' if self.debug else 'Release'

		cmake_args = [
			f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}',
			f'-DPython3_EXECUTABLE={sys.executable}',
		]
		build_args = ['--config', cfg, '-j', '8']

		if platform.system() == 'Windows':
			cmake_args += [f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}']
			cmake_args += ['-A', 'x64' if sys.maxsize > 2**32 else 'Win32']
			build_args += ['--', '/m']
		else:
			cmake_args += [f'-DCMAKE_BUILD_TYPE={cfg}']

		os.makedirs(self.build_temp, exist_ok=True)
		subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp)
		subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)


setup(
	ext_modules=[CMakeExtension('zxingcpp')],
	cmdclass={'build_ext': CMakeBuild},
)
