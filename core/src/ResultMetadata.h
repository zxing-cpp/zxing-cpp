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

#include <string>
#include <map>
#include <memory>
#include <list>

namespace ZXing {

class ByteArray;
class CustomData;

/**
* @author Sean Owen
*/
class ResultMetadata
{
public:
	
	/**
	* Represents some type of metadata about the result of the decoding that the decoder
	* wishes to communicate back to the caller.
	*/
	enum Key {

		/**
		* Unspecified, application-specific metadata. Maps to an unspecified {@link CustomData}.
		*/
		OTHER,

		/**
		* Denotes the likely approximate orientation of the barcode in the image. This value
		* is given as degrees rotated clockwise from the normal, upright orientation.
		* For example a 1D barcode which was found by reading top-to-bottom would be
		* said to have orientation "90". This key maps to an {@link Integer} whose
		* value is in the range [0,360).
		*/
		ORIENTATION,

		/**
		* <p>2D barcode formats typically encode text, but allow for a sort of 'byte mode'
		* which is sometimes used to encode binary data. While {@link Result} makes available
		* the complete raw bytes in the barcode for these formats, it does not offer the bytes
		* from the byte segments alone.</p>
		*
		* <p>This maps to a {@link java.util.List} of byte arrays corresponding to the
		* raw bytes in the byte segments in the barcode, in order.</p>
		*/
		BYTE_SEGMENTS,

		/**
		* Error correction level used, if applicable. The value type depends on the
		* format, but is typically a String.
		*/
		ERROR_CORRECTION_LEVEL,

		/**
		* For some periodicals, indicates the issue number as an {@link Integer}.
		*/
		ISSUE_NUMBER,

		/**
		* For some products, indicates the suggested retail price in the barcode as a
		* formatted {@link String}.
		*/
		SUGGESTED_PRICE,

		/**
		* For some products, the possible country of manufacture as a {@link String} denoting the
		* ISO country code. Some map to multiple possible countries, like "US/CA".
		*/
		POSSIBLE_COUNTRY,

		/**
		* For some products, the extension text
		*/
		UPC_EAN_EXTENSION,

		/**
		* PDF417-specific metadata
		*/
		PDF417_EXTRA_METADATA,

		/**
		* If the code format supports structured append and the current scanned code is part of one then the
		* sequence number is given with it.
		*/
		STRUCTURED_APPEND_SEQUENCE,

		/**
		* If the code format supports structured append and the current scanned code is part of one then the
		* parity is given with it.
		*/
		STRUCTURED_APPEND_PARITY,

	};

	int getInt(Key key, int fallbackValue = 0) const;
	std::wstring getString(Key key) const;
	std::list<ByteArray> getByteArrayList(Key key) const;
	std::shared_ptr<CustomData> getCustomData(Key key) const;
	
	void put(Key key, int value);
	void put(Key key, const std::wstring& value);
	void put(Key key, const std::list<ByteArray>& value);
	void put(Key key, const std::shared_ptr<CustomData>& value);

	void putAll(const ResultMetadata& other);

private:
	struct Value;
	struct IntegerValue;
	struct StringValue;
	struct ByteArrayListValue;
	struct CustomDataValue;

	std::map<Key, std::shared_ptr<Value>> _contents;
};

} // ZXing
