#pragma once

#include <string>
#include <chrono>

namespace ssl_helpers {

std::string to_hex(const std::string&);
std::string from_hex(const std::string&);

//with '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz' alphabet (Bitcoin style)
std::string to_base58(const std::string& data);
std::string from_base58(const std::string& str);

std::string to_base58(const char* data, size_t len);
std::string from_base58(const char* str, size_t len);

std::string to_base64(const std::string& data);
std::string from_base64(const std::string& str);

std::string to_base64(const char* data, size_t len);
std::string from_base64(const char* str, size_t len);

} // namespace ssl_helpers
