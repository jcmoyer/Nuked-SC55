#pragma once

#include <filesystem>
#include <iosfwd>
#include <vector>

bool FIO_ReadAllBytes(const std::filesystem::path& filename, std::vector<uint8_t>& buffer);

bool            FIO_ReadStreamExact(std::ifstream& s, void* into, std::streamsize byte_count);
bool            FIO_ReadStreamExact(std::ifstream& s, std::span<uint8_t> into, std::streamsize byte_count);
std::streamsize FIO_ReadStreamUpTo(std::ifstream& s, void* into, std::streamsize byte_count);
