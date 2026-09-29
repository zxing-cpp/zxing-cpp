/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BitMatrix.h"
#include "Matrix.h"
#include "Point.h"

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <string>

namespace ZXing {

enum LogColor : int8_t {
	LOG_R = 4, // red
	LOG_G = 2, // green
	LOG_B = 3, // blue
	LOG_GR = 1, // gray
	LOG_I = -1, // inverted
};

#ifdef PRINT_DEBUG

class LogMatrix
{
	using LogBuffer = Matrix<LogColor>;
	LogBuffer _log;
	const BitMatrix* _image = nullptr;
	int _scale = 1;

public:
	void init(const BitMatrix* image, int scale = 1)
	{
		_image = image;
		_scale = scale;
		_log = LogBuffer(_image->width() * _scale, _image->height() * _scale);
	}

	void write(const char* fn)
	{
		assert(_image);
		FILE* f = fopen(fn, "wb");

		// Write PPM header, P5 == grey, P6 == rgb
		fprintf(f, "P6\n%d %d\n255\n", _log.width(), _log.height());

		// Write pixels
		for (int y = 0; y < _log.height(); ++y)
			for (int x = 0; x < _log.width(); ++x) {
				unsigned char pix[3];
				unsigned char &r = pix[0], &g = pix[1], &b = pix[2];
				r = g = b = _image->get(x / _scale, y / _scale) ? 0 : 255;
				if (_scale > 1 && x % _scale == _scale / 2 && y % _scale == _scale / 2)
					r = g = b = r ? 240 : 45;
				switch (_log.get(x, y)) {
				case LOG_I: r = g = b = _scale > 1 ? 255 - r : (r ? 50 : 230); break;
				case LOG_GR: r = g = b = _scale > 1 ? 128 : (r ? 230 : 50); break;
				case LOG_G: r = b = 50, g = 220; break;
				case LOG_B: g = r = 100, b = 250; break;
				case LOG_R: g = b = 100, r = 250; break;
				}
				fwrite(&pix, 3, 1, f);
			}
		fclose(f);
	}

	template <typename T>
	void operator()(const PointT<T>& p, LogColor color = LOG_GR, int size = 1)
	{
		if (_image && _image->isIn(p)) {
			for (int dy = -size / 2; dy <= size / 2; ++dy)
				_log.set(static_cast<int>(p.x * _scale), static_cast<int>(p.y * _scale) + dy, color);
			for (int dx = -size / 2; dx <= size / 2; ++dx)
				_log.set(static_cast<int>(p.x * _scale) + dx, static_cast<int>(p.y * _scale), color);
		}
	}

	void operator()(const PointT<int>& p, LogColor color, int size = 1)
	{
		operator()(centered(p), color, size);
	}

	template <typename T>
	void operator()(const std::vector<PointT<T>>& points, LogColor color = LOG_G)
	{
		for (auto p : points)
			operator()(p, color);
	}

	template <typename T>
	void operator()(const PointT<T>& a, const PointT<T>& b, LogColor color = LOG_G, int density = 1)
	{
		int steps = density * maxAbsComponent(b - a);
		PointF dir = 1. / density * bresenhamDirection(PointF(b - a));
		for (int i = 0; i <= steps; ++i)
			operator()(a + i * dir, color);
	}
};

extern LogMatrix log;

class LogMatrixWriter
{
	LogMatrix &log;
	std::string fn;

public:
	LogMatrixWriter(LogMatrix& log, const BitMatrix& image, int scale, std::string fn) : log(log), fn(fn)
	{
		log.init(&image, scale);
	}
	~LogMatrixWriter() { log.write(fn.c_str()); }
};

/// @brief Logs text to stdout, no newline
inline void log_t(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::vfprintf(stdout, fmt, args);
	va_end(args);
}

/// @brief Logs text to stdout with a newline at the end.
inline void log_l(const char* fmt = "", ...)
{
	va_list args;
	va_start(args, fmt);
	std::vfprintf(stdout, fmt, args);
	va_end(args);
	std::fputc('\n', stdout);
}

/// @brief Logs a range of values to stdout with a prefix and postfix string.
template<typename Range>
void log_r(const char* prefix, const char* fmt, const Range& values, const char* postfix = "\n")
{
	log_t("%s", prefix);
	for (const auto& v : values)
		log_t(fmt, v);
	log_t("%s", postfix);
}

#else

template<typename T> void log(PointT<T>, LogColor = LOG_GR, int = 1) {}
template<typename T> void log(PointT<T>, PointT<T>, LogColor = LOG_GR, int = 1) {}
inline void log_t(const char*, ...) {}
inline void log_l(const char* = "", ...) {}
template<typename Range> void log_r(const char*, const char*, const Range&, const char* = "\n") {}

#endif

} // namespace ZXing
