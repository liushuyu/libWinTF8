/*
  Copyright (c) 2014 StarBrilliant <m13253@hotmail.com>
  All rights reserved.

  Redistribution and use in source and binary forms are permitted
  provided that the above copyright notice and this paragraph are
  duplicated in all such forms and that any documentation,
  advertising materials, and other materials related to such
  distribution and use acknowledge that the software was developed by
  StarBrilliant.
  The name of StarBrilliant may not be used to endorse or promote
  products derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <vector>
#include "utils.h"
#include "termio.h"
#include "printf.h"

namespace WTF8 {

#ifdef _WIN32
int vasprintf(std::vector<char> &result, const char *format, va_list ap, size_t size_hint = 256) {
    result.clear();
    result.resize(size_hint);
    int size;
    va_list ap_copy;
    va_copy(ap_copy, ap);
    size = vsnprintf(result.data(), result.size(), format, ap_copy);
    va_end(ap_copy);
    if(size < 0)
        result.resize(0);
    else if(size_t(size) >= size_hint) {
        result.clear();
        result.resize(size+1);
        va_copy(ap_copy, ap);
        size = vsnprintf(result.data(), result.size(), format, ap_copy);
        va_end(ap_copy);
    }
    return size;
}
#endif

int printf(const char *format, ...) {
    va_list ap;
    int size;
    va_start(ap, format);
    size = WTF8::vprintf(format, ap);
    va_end(ap);
    return size;
}

int vprintf(const char *format, va_list ap) {
#ifdef _WIN32
    std::vector<char> result;
    int size = WTF8::vasprintf(result, format, ap);
    if(size > 0)
        if(!cout.write(result.data(), size))
            return -1;
    return size;
#else
    return std::vprintf(format, ap);
#endif
}

int fprintf(std::FILE *stream, const char *format, ...) {
    va_list ap;
    int size;
    va_start(ap, format);
    size = WTF8::vfprintf(stream, format, ap);
    va_end(ap);
    return size;
}

int vfprintf(std::FILE *stream, const char *format, va_list ap) {
#ifdef _WIN32
    if(stream == stdout || stream == stderr) {
        std::vector<char> result;
        int size = WTF8::vasprintf(result, format, ap);
        if(size > 0) {
            if(stream != stderr) {
                if(!cout.write(result.data(), size))
                    return -1;
            } else {
                if(!cerr.write(result.data(), size))
                    return -1;
            }
        }
        return size;
    } else
        return std::vfprintf(stream, format, ap);
#else
    return std::vfprintf(stream, format, ap);
#endif
}

}

extern "C" {

int WTF8_printf(const char *format, ...) {
    va_list ap;
    int size;
    va_start(ap, format);
    size = WTF8::vprintf(format, ap);
    va_end(ap);
    return size;
}

int WTF8_vprintf(const char *format, va_list ap) {
    return WTF8::vprintf(format, ap);
}

int WTF8_fprintf(std::FILE *stream, const char *format, ...) {
    va_list ap;
    int size;
    va_start(ap, format);
    size = WTF8::fprintf(stream, format, ap);
    va_end(ap);
    return size;
}

int WTF8_vfprintf(std::FILE *stream, const char *format, va_list ap) {
    return WTF8::fprintf(stream, format, ap);
}

}
