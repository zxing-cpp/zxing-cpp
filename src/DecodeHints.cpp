/*
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

#include "DecodeHints.h"
#include "ZXString.h"
#include "BarcodeFormat.h"

namespace ZXing {

struct DecodeHints::HintValue
{
	virtual ~HintValue() {}
	virtual bool toBoolean() const {
		return false;
	}
	virtual String toString() const {
		return String();
	}
	virtual std::vector<int> toIntegerList() const {
		return std::vector<int>();
	}
	virtual std::vector<BarcodeFormat> toFormatList() const {
		return std::vector<BarcodeFormat>();
	}
	virtual DecodeHints::PointCallback toPointCallback() const {
		return nullptr;
	}
};

struct DecodeHints::BooleanHintValue : public HintValue
{
	bool value;
	BooleanHintValue(bool v) : value(v) {}
	virtual bool toBoolean() const override {
		return value;
	}
};

struct DecodeHints::StringHintValue : public HintValue
{
	String value;
	StringHintValue(const String& v) : value(v) {}
	virtual String toString() const override {
		return value;
	}
};

struct DecodeHints::IntegerListValue : public HintValue
{
	std::vector<int> value;
	IntegerListValue(const std::vector<int>& v) : value(v) {}
	virtual std::vector<int> toIntegerList() const override {
		return value;
	}
};

struct DecodeHints::FormatListValue : public HintValue
{
	std::vector<BarcodeFormat> value;
	FormatListValue(const std::vector<BarcodeFormat>& v) : value(v) {}
	virtual std::vector<BarcodeFormat> toFormatList() const override {
		return value;
	}
};

struct DecodeHints::PointCallbackValue : public HintValue
{
	DecodeHints::PointCallback value;
	PointCallbackValue(const DecodeHints::PointCallback& v) : value(v) {}
	virtual DecodeHints::PointCallback toPointCallback() const override {
		return value;
	}
};


bool
DecodeHints::getFlag(DecodeHint hint) const
{
	auto it = _contents.find(hint);
	return it != _contents.end() ? it->second->toBoolean() : false;
}

String
DecodeHints::getString(DecodeHint hint) const
{
	auto it = _contents.find(hint);
	return it != _contents.end() ? it->second->toString() : String();
}

std::vector<int>
DecodeHints::getIntegerList(DecodeHint hint) const
{
	auto it = _contents.find(hint);
	return it != _contents.end() ? it->second->toIntegerList() : std::vector<int>();
}

std::vector<BarcodeFormat>
DecodeHints::getFormatList(DecodeHint hint) const
{
	auto it = _contents.find(hint);
	return it != _contents.end() ? it->second->toFormatList() : std::vector<BarcodeFormat>();
}

DecodeHints::PointCallback
DecodeHints::getPointCallback(DecodeHint hint) const
{
	auto it = _contents.find(hint);
	return it != _contents.end() ? it->second->toPointCallback() : nullptr;
}

void
DecodeHints::put(DecodeHint hint, bool value)
{
	_contents[hint] = std::make_shared<BooleanHintValue>(value);
}

void
DecodeHints::put(DecodeHint hint, const String& value)
{
	_contents[hint] = std::make_shared<StringHintValue>(value);
}

void
DecodeHints::put(DecodeHint hint, const std::vector<int>& list)
{
	_contents[hint] = std::make_shared<IntegerListValue>(list);
}

void
DecodeHints::put(DecodeHint hint, const std::vector<BarcodeFormat>& formats)
{
	_contents[hint] = std::make_shared<FormatListValue>(formats);
}

void
DecodeHints::put(DecodeHint hint, const PointCallback& callback)
{
	_contents[hint] = std::make_shared<PointCallbackValue>(callback);
}

void
DecodeHints::remove(DecodeHint hint)
{
	_contents.erase(hint);
}

} // ZXing
