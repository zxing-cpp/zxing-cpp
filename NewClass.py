import sys, os

def writeToFile(path, text):
	print('Create file "%s"' % path)
	with open(path, 'w', encoding='utf-8') as f:
		f.write(text) # Write a string to a file

def write_files(relPath, fileName, className, namespaces):	
	fileContents = '\
#pragma once\n\
/*\n\
* Copyright 2016 ZXing authors\n\
*\n\
* Licensed under the Apache License, Version 2.0 (the "License");\n\
* you may not use this file except in compliance with the License.\n\
* You may obtain a copy of the License at\n\
*\n\
*      http://www.apache.org/licenses/LICENSE-2.0\n\
*\n\
* Unless required by applicable law or agreed to in writing, software\n\
* distributed under the License is distributed on an "AS IS" BASIS,\n\
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n\
* See the License for the specific language governing permissions and\n\
* limitations under the License.\n\
*/\n\
\n\
namespace ZXing {\n';
	for ns in namespaces:
		fileContents += 'namespace ' + ns + ' {\n'

	fileContents += '\nclass ' + className + '\n{\n};\n\n'

	for ns in reversed(namespaces):
		fileContents += '} // ' + ns + '\n'
	
	fileContents += '} // ZXing\n'

	writeToFile(fileName + '.h', fileContents)
	
	fileContents = '\
/*\n\
* Copyright 2016 ZXing authors\n\
*\n\
* Licensed under the Apache License, Version 2.0 (the "License");\n\
* you may not use this file except in compliance with the License.\n\
* You may obtain a copy of the License at\n\
*\n\
*      http://www.apache.org/licenses/LICENSE-2.0\n\
*\n\
* Unless required by applicable law or agreed to in writing, software\n\
* distributed under the License is distributed on an "AS IS" BASIS,\n\
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n\
* See the License for the specific language governing permissions and\n\
* limitations under the License.\n\
*/\n\
\n\
#include "' + relPath + fileName + '.h' + '"\n\
\n\
namespace ZXing {\n';
	for ns in namespaces:
		fileContents += 'namespace ' + ns + ' {\n'

	fileContents += '\n\n\n'

	for ns in reversed(namespaces):
		fileContents += '} // ' + ns + '\n'
	
	fileContents += '} // ZXing\n'

	writeToFile(fileName + '.cpp', fileContents)

if __name__ == '__main__':
	try:
		dirs = os.getcwd().split(os.path.sep)
		relPath = '/'.join(dirs[dirs.index('src')+1:])
	except ValueError:
		relPath = ''
	
	if len(relPath) > 0:
		relPath += '/'
	
	names = sys.argv[1].split('-')
	prefix = ''
	if len(names) > 1:
		prefix = names[0]
		names[0] = names[1]
	names = names[0].split('::');

	write_files(relPath, prefix + names[-1], names[-1], names[:-1])
