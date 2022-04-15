#pragma once

#include <string>


std::string base64_encode(const char* s, size_t len);
std::string base64_decode(const char* s, size_t len);

size_t base64_decode_buffer(char* dest, size_t destsize, const char* src, size_t srcsize);