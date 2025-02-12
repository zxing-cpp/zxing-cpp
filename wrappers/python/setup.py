import json
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
	def build_extension(self, ext):
		extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
		cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
					  '-DPython3_EXECUTABLE=' + sys.executable,
					  '-DVERSION_INFO=' + self.distribution.get_version()]

		cfg = 'Debug' if self.debug else 'Release'
		build_args = ['--config', cfg,
					  '-j', '8']

		if platform.system() == "Windows":
			cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
			if sys.maxsize > 2**32:
				cmake_args += ['-A', 'x64']
			else:
				cmake_args += ['-A', 'Win32']
			build_args += ['--', '/m']
		else:
			cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]

		if not os.path.exists(self.build_temp):
			os.makedirs(self.build_temp)

		subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp)
		subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)


def get_setup_requires():
	try:
		subp = subprocess.run(['cmake', '-E', 'capabilities'], stdout=subprocess.PIPE)
	except OSError:
		pass
	else:
		if subp.returncode == 0:
			version = json.loads(subp.stdout).get('version', {})
			version_split = (version.get('major', 0), version.get('minor', 0))
			if version_split >= (3, 15):
				return []
	return ['cmake>=3.15']


with open("README.md", "r", encoding="utf-8") as fh:
	long_description = fh.read()


setup(
	name='zxing-cpp',
	# setuptools_scm cannot be used because of the structure of the project until the following issues are solved:
	# https://github.com/pypa/setuptools_scm/issues/357
	# https://github.com/pypa/pip/issues/7549
	# Because pip works on a copy of current directory in a temporary directory, the temporary directory does not hold
	# the .git directory of the repo, so that setuptools_scm cannot guess the current version.
	# use_scm_version={
	#     "root": "../..",
	#     "version_scheme": "guess-next-dev",
	#     "local_scheme": "no-local-version",
	#     "tag_regex": "v?([0-9]+.[0-9]+.[0-9]+)",
	# },
	version='2.3.0',
	description='Python bindings for the zxing-cpp barcode library',
	long_description=long_description,
	long_description_content_type="text/markdown",
	author='ZXing-C++ Community',
	author_email='zxingcpp@gmail.com',
	url='https://github.com/zxing-cpp/zxing-cpp',
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
	setup_requires=get_setup_requires(),
	ext_modules=[CMakeExtension('zxingcpp')],
	cmdclass=dict(build_ext=CMakeBuild),
	zip_safe=False,
)
