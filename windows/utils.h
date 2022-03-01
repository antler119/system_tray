#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

#include <flutter/encodable_value.h>

namespace utils {

// Converts the given UTF-8 string to UTF-16.
std::wstring Utf16FromUtf8(const std::string& utf8_string);

const flutter::EncodableValue* ValueOrNull(const flutter::EncodableMap& map,
                                           const char* key);

}  // namespace utils

#endif  // __UTILS_H__