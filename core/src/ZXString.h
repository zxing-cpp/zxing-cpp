#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include <string>
#include <cstdint>
#include <iosfwd>
#include <iterator>

#ifdef ZX_HAVE_QT
	class QString;
#endif

namespace ZXing {

/// <summary>
/// This is UTF-8 string class (i.e. it keeps data internally as UTF-8).
/// Note that this class does not support null character in middle of string.
/// </summary>
class String
{
public:
	class Iterator : public std::iterator<std::input_iterator_tag, uint32_t>
	{
	public:
		Iterator(std::string::const_iterator p, std::string::const_iterator e) : m_ptr(p), m_end(e) {}

		Iterator& operator++()
		{
			next();
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator it = *this;
			next();
			return it;
		}

		uint32_t operator*() const
		{
			return read();
		}

		bool operator!=(const Iterator& other) const {
			return m_ptr != other.m_ptr;
		}

		bool operator==(const Iterator& other) const {
			return m_ptr == other.m_ptr;
		}
	private:
		std::string::const_iterator m_ptr;
		std::string::const_iterator m_end;
		void next();
		uint32_t read() const;
	};

	String() {}
	String(const std::string& other) : m_utf8(other) {;}
	String(const char* i_utf8, int i_len = -1) {
		if (i_utf8 != nullptr) {
			if (i_len >= 0) {
				m_utf8.append(i_utf8, i_len);
			}
			else {
				m_utf8.append(i_utf8);
			}
		}
	}

	String(const wchar_t* i_wstr);
	String(const wchar_t* i_begin, const wchar_t* i_end);
	String(const wchar_t* i_wstr, size_t i_len);

	bool empty() const											{ return m_utf8.empty(); }
	int byteCount() const										{ return static_cast<int>(m_utf8.length()); }
	int charCount() const;	// count the number of characters (which may not same as byte count)

	// index is char index not byte index
	uint32_t charAt(int charIndex) const;
	String substring(int charIndex, int charCount = -1) const;

	Iterator begin() const;
	Iterator end() const;

	void appendUtf8(char c)										{ m_utf8.append(1, c); }
	void appendUtf8(const char* str)							{ m_utf8.append(str); }
	void appendUtf8(const char* str, size_t len)				{ m_utf8.append(str, len); }
	void appendUtf8(const std::string& str)						{ m_utf8.append(str.data(), static_cast<int>(str.length())); }
	void appendUtf8(const uint8_t* str)							{ m_utf8.append((const char*)str); }
	void appendUtf8(const uint8_t* str, size_t len)				{ m_utf8.append((const char*)str, len); }
	void appendUcs2(const uint16_t* ucs2);
	void appendUcs2(const uint16_t* ucs2, size_t len);
	void appendUtf16(const uint16_t* utf16, size_t len);
	void appendUtf16(const std::vector<uint16_t>& utf16);
	void appendUtf32(uint32_t utf32);
	void appendUtf32(const uint32_t* utf32, size_t len);
	void appendUtf32(const std::vector<uint32_t>& utf32);

	void appendLatin1(const uint8_t* str, size_t len);
	void appendLatin1(const char* str, size_t len)				{ appendLatin1((const uint8_t*)str, len); }
	void appendLatin1(const std::string& str)					{ appendLatin1(str.data(), str.length()); }

	void prependUtf8(char c)									{ m_utf8.insert(m_utf8.begin(), c); }

	const char* utf8() const { return m_utf8.c_str(); }
	void toUtf16(std::vector<uint16_t>& buffer) const;
	void toUtf32(std::vector<uint32_t>& buffer) const;
	void toWString(std::vector<wchar_t>& buffer) const;
	std::wstring toWString() const;

	std::string toStdString() const
	{
		return m_utf8;
	}

	static String FromLatin1(const std::string& latin1)			{ String s; s.appendLatin1(latin1); return s; }
	static String FromLatin1(const uint8_t* str, size_t len)	{ String s; s.appendLatin1(str, len); return s; }
	static String FromLatin1(const char* str, size_t len)		{ String s; s.appendLatin1(str, len); return s; }

	bool operator==(const String& other) const
	{
		return m_utf8 == other.m_utf8;
	}

	bool operator!=(const String& other) const
	{
		return m_utf8 != other.m_utf8;
	}

	bool operator<(const String& other) const
	{
		return m_utf8 < other.m_utf8; // this is a property of UTF-8
	}

	bool operator<=(const String& other) const
	{
		return m_utf8 <= other.m_utf8; // this is a property of UTF-8
	}

	bool operator>(const String& other) const
	{
		return m_utf8 > other.m_utf8; // this is a property of UTF-8
	}

	bool operator>=(const String& other) const
	{
		return m_utf8 >= other.m_utf8; // this is a property of UTF-8
	}
	
	String operator+(const String& other) const
	{
		String sum(*this);
		return sum += other;
	}

	String& operator+=(const String& other)
	{
		m_utf8.append(other.m_utf8.begin(), other.m_utf8.end());
		return *this;
	}
	
#ifdef NB_HAVE_QT
	String(const QString& qstr);
	operator QString() const;
#endif

	friend std::ostream& operator<<(std::ostream& out, const String& str);
	friend std::wostream& operator<<(std::wostream& out, const String& str);

private:
	std::string m_utf8;
};

} // namespace ZXing
