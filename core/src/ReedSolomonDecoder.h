#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <vector>

namespace ZXing {

class GenericGF;

/**
* <p>Implements Reed-Solomon decoding, as the name implies.</p>
*
* <p>The algorithm will not be explained here, but the following references were helpful
* in creating this implementation:</p>
*
* <ul>
* <li>Bruce Maggs.
* <a href="http://www.cs.cmu.edu/afs/cs.cmu.edu/project/pscico-guyb/realworld/www/rs_decode.ps">
* "Decoding Reed-Solomon Codes"</a> (see discussion of Forney's Formula)</li>
* <li>J.I. Hall. <a href="www.mth.msu.edu/~jhall/classes/codenotes/GRS.pdf">
* "Chapter 5. Generalized Reed-Solomon Codes"</a>
* (see discussion of Euclidean algorithm)</li>
* </ul>
*
* <p>Much credit is due to William Rucklidge since portions of this code are an indirect
* port of his C++ Reed-Solomon implementation.</p>
*
* @author Sean Owen
* @author William Rucklidge
* @author sanfordsquires
*/
class ReedSolomonDecoder
{
public:
	/**
	* <p>Decodes given set of received codewords, which include both data and error-correction
	* codewords. Really, this means it uses Reed-Solomon to detect and correct errors, in-place,
	* in the input.</p>
	*
	* @param received data and error-correction codewords
	* @param twoS number of error-correction codewords available
	* @throws ReedSolomonException if decoding fails for any reason
	*/
	static bool Decode(const GenericGF& field, std::vector<int>& received, int twoS);
};

} // ZXing
