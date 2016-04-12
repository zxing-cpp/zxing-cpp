#include "ZXString.h"
#include "ZXUtf16.h"
#include "ZXUtf8.h"

#ifdef NB_HAVE_QT
#include <qstring.h>
#endif

#include <iostream>

namespace ZXing {

namespace {
	
	template <typename Iter>
	void stringAppendUtf16(std::string& str, Iter beginRange, Iter endRange)
	{
		char buffer[6];
		int bufLength;
		
		while (beginRange != endRange)
		{
			uint32_t codePoint = *beginRange++;
	
			if (Utf16::IsHighSurrogate(codePoint) && beginRange != endRange && Utf16::IsLowSurrogate(*beginRange))
			{
				uint32_t nextCodePoint = *beginRange++;
				bufLength = Utf8::Encode(Utf16::CodePointFromSurrogates(codePoint, nextCodePoint), buffer);
			}
			else
			{
				bufLength = Utf8::Encode(codePoint, buffer);
			}
			str.append(buffer, bufLength);
		}
	}

	template <typename Iter>
	void stringAppendUtf32(std::string& str, Iter beginRange, Iter endRange)
	{
		char buffer[6];
		int bufLength;
		while (beginRange != endRange)
		{
			bufLength = Utf8::Encode(*beginRange++, buffer);
			str.append(buffer, bufLength);
		}
	}

	template <typename Container>
	void stringToUtf16(const std::string& str, Container& buffer)
	{
		typedef typename Container::value_type CharType;

		const uint8_t* src = (const uint8_t*)str.c_str();
		const uint8_t* srcEnd = src + str.size();
		int destLen = Utf8::CountCodePoints((const char*)src);
		if (destLen > 0)
		{
			buffer.reserve(buffer.size() + destLen);
			uint32_t codePoint;
			uint32_t state = 0;

			while (src < srcEnd)
			{
				if (Utf8::Decode(*src++, state, codePoint) != Utf8::kAccepted)
				{
					continue;
				}

				if (codePoint > 0xffff) // surrogate pair
				{
					buffer.push_back((CharType)(0xd7c0 + (codePoint >> 10)));
					buffer.push_back((CharType)(0xdc00 + (codePoint & 0x3ff)));
				}
				else
				{
					buffer.push_back((CharType)codePoint);
      			}
	
			}
		}
	}

	template <typename Container>
	void stringToUtf32(const std::string& str, Container& buffer)
	{
		const uint8_t* src = (const uint8_t*)str.c_str();
		const uint8_t* srcEnd = src + str.size();
		int destLen = Utf8::CountCodePoints((const char*)src);
		if (destLen > 0)
		{
			buffer.reserve(buffer.size() + destLen);
			uint32_t codePoint;
			uint32_t state = 0;

			while (src < srcEnd)
			{
				if (Utf8::Decode(*src++, state, codePoint) != Utf8::kAccepted)
				{
					continue;
				}
				buffer.push_back((typename Container::value_type)codePoint);
			}
		}
	}


};

String::String(const wchar_t* i_wstr)
{
	if (sizeof(wchar_t) == 2)
	{
		size_t len = std::char_traits<wchar_t>::length(i_wstr);
		stringAppendUtf16(m_utf8, i_wstr, i_wstr + len);
	}
	else
	{
		char buffer[6];
		for (; *i_wstr; ++i_wstr)
		{
			int length = Utf8::Encode(static_cast<uint32_t>(*i_wstr), buffer);
			m_utf8.append(buffer, length);
		}
	}
}

String::String(const wchar_t* i_begin, const wchar_t* i_end)
{
	if (sizeof(wchar_t) == 2)
	{
		stringAppendUtf16(m_utf8, i_begin, i_end);
	}
	else
	{
		stringAppendUtf32(m_utf8, i_begin, i_end);
	}
}

String::String(const wchar_t* i_wstr, int i_len)
{
	if (sizeof(wchar_t) == 2)
	{
		stringAppendUtf16(m_utf8, i_wstr, i_wstr + i_len);
	}
	else
	{
		stringAppendUtf32(m_utf8, i_wstr, i_wstr + i_len);
	}
}

#ifdef NB_HAVE_QT

String::String(const QString& qstr)
{
	const ushort* utf16 = qstr.utf16();
	int len = std::char_traits<ushort>::length(utf16);
	stringAppendUtf16(m_utf8, utf16, utf16 + len);
}

String::operator QString() const
{
	return QString::fromUtf8(m_utf8.c_str(), m_utf8.length());
}

#endif

int
String::charCount() const
{
	return Utf8::CountCodePoints(m_utf8.c_str());
}

void
String::appendUcs2(const uint16_t* ucs2, int len)
{
	char buffer[6];
	for (int i = 0; i < len; ++i)
	{
		int length = Utf8::Encode(static_cast<uint32_t>(ucs2[i]), buffer);
		m_utf8.append(buffer, length);
	}
}

void
String::appendUcs2(const uint16_t* ucs2)
{
	char buffer[6];
	for (; *ucs2; ++ucs2)
	{
		int length = Utf8::Encode(static_cast<uint32_t>(*ucs2), buffer);
		m_utf8.append(buffer, length);
	}
}

void
String::appendUtf16(const std::vector<uint16_t>& utf16)
{
	stringAppendUtf16(m_utf8, utf16.begin(), utf16.end());
}

void
String::appendUtf32(const std::vector<uint32_t>& utf32)
{
	stringAppendUtf32(m_utf8, utf32.begin(), utf32.end());
}

void
String::appendUtf16(const uint16_t* utf16, int len)
{
	stringAppendUtf16(m_utf8, utf16, utf16 + len);
}

void
String::appendUtf32(const uint32_t* utf32, int len)
{
	stringAppendUtf32(m_utf8, utf32, utf32 + len);
}

void
String::toUtf16(std::vector<uint16_t>& buffer) const
{
	stringToUtf16(m_utf8, buffer);
}

void
String::toUtf32(std::vector<uint32_t>& buffer) const
{
	stringToUtf32(m_utf8, buffer);
}

void
String::toWString(std::vector<wchar_t>& buffer) const
{
	if (sizeof(wchar_t) == 2)
	{
		stringToUtf16(m_utf8, buffer);
	}
	else
	{
		stringToUtf32(m_utf8, buffer);
	}
}

std::wstring
String::toWString() const
{
	std::wstring buffer;
	if (sizeof(wchar_t) == 2)
	{
		stringToUtf16(m_utf8, buffer);
	}
	else
	{
		stringToUtf32(m_utf8, buffer);
	}
	return buffer;
}

std::ostream &
operator<<(std::ostream& out, const String& str)
{
	return out.write(str.m_utf8.data(), str.m_utf8.size());
}

std::wostream &
operator<<(std::wostream& out, const String& str)
{
	return (out << str.toWString());
}

} // ZXing