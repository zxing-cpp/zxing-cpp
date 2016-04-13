#pragma once
#include <exception>
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
*/
class ReedSolomonDecoder
{
public:
	explicit ReedSolomonDecoder(const GenericGF& field) : _field(&field) {}

	/**
	* <p>Decodes given set of received codewords, which include both data and error-correction
	* codewords. Really, this means it uses Reed-Solomon to detect and correct errors, in-place,
	* in the input.</p>
	*
	* @param received data and error-correction codewords
	* @param twoS number of error-correction codewords available
	* @throws ReedSolomonException if decoding fails for any reason
	*/
	bool decode(std::vector<int>& received, int twoS) const;

private:
	const GenericGF* _field;
};

//class ReedSolomonException : public std::exception
//{
//public:
//	ReedSolomonException(const char* msg) : std::exception(msg) {}
//};

} // ZXing