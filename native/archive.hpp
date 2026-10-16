#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace actua {
using Bytes = std::vector<std::uint8_t>;
struct Entry { std::uint32_t offset, size; };
class Archive {
public:
    Archive(const std::filesystem::path& data, const std::filesystem::path& offsets);
    Archive(Bytes data, std::span<const std::uint8_t> offsets);
    std::span<const std::uint8_t> entry(std::size_t index) const;
    std::size_t count() const { return entries_.size(); }
    std::size_t size() const { return data_.size(); }
private:
    Bytes data_;
    std::vector<Entry> entries_;
};
Bytes read_file(const std::filesystem::path& path);
Bytes decode_screen(const Archive& archive, std::size_t screen);
struct Point { std::int32_t x, y; };
using Zone = std::array<Point, 10>;
using Tactics = std::array<Zone, 70>;
Tactics decode_tactics(std::span<const std::uint8_t> bytes);
extern const std::array<const char*, 15> screen_names;
extern const std::array<const char*, 10> formation_names;
}
// Original function bodies extracted unchanged during the build.
using BYTE = unsigned char;
void straightcopy(int count, BYTE* dest, BYTE* src);
void spritecopy(int count, BYTE* dest, BYTE* src);
void flipsprcopy(int count, BYTE* dest, BYTE* src);
void pseudostore(int count, BYTE* dest, BYTE* src);
float calc_dist(float x, float y);

