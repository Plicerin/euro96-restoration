#include "archive.hpp"
#include <algorithm>
#include <bit>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace actua {
const std::array<const char*, 15> screen_names = {
    "Main menu", "Team setup", "Group fixtures", "Team selection", "Tournament draw",
    "Player statistics", "Player setup", "Team statistics", "Venue",
    "Modem setup", "Next match", "Qualifying", "Quarter-final", "Semi-final", "Final"};
const std::array<const char*, 10> formation_names = {
    "3-1-6", "4-0-6", "4-1-5", "4-2-4", "4-3-3", "4-4-2", "5-1-4", "5-2-3", "5-3-2", "6-3-1"};
static std::uint32_t u32(std::span<const std::uint8_t> b, std::size_t at) {
    return std::uint32_t(b[at]) | (std::uint32_t(b[at+1]) << 8) |
        (std::uint32_t(b[at+2]) << 16) | (std::uint32_t(b[at+3]) << 24);
}
Bytes read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) throw std::runtime_error("Cannot open asset: " + path.filename().string());
    const auto end = stream.tellg();
    if (end < 0 || end > 128 * 1024 * 1024)
        throw std::runtime_error("Invalid or oversized asset: " + path.filename().string());
    Bytes data(static_cast<std::size_t>(end));
    stream.seekg(0);
    if (!data.empty() && !stream.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size())))
        throw std::runtime_error("Incomplete asset read: " + path.filename().string());
    return data;
}
Archive::Archive(const std::filesystem::path& data, const std::filesystem::path& offsets)
    : Archive(read_file(data), read_file(offsets)) {}
Archive::Archive(Bytes data, std::span<const std::uint8_t> offsets) : data_(std::move(data)) {
    if (offsets.empty() || offsets.size() % 8 != 0)
        throw std::runtime_error("Offset table must contain complete 8-byte records");
    for (std::size_t i=0; i<offsets.size(); i+=8) {
        Entry e{u32(offsets,i),u32(offsets,i+4)};
        // Subtraction prevents overflow on corrupt 32-bit offsets.
        if (e.offset > data_.size() || e.size > data_.size() - e.offset)
            throw std::runtime_error("Archive entry " + std::to_string(i/8) + " exceeds data bounds");
        entries_.push_back(e);
    }
}
std::span<const std::uint8_t> Archive::entry(std::size_t i) const {
    if (i >= entries_.size()) throw std::out_of_range("Archive entry index");
    return std::span<const std::uint8_t>(data_).subspan(entries_[i].offset,entries_[i].size);
}
Bytes decode_screen(const Archive& archive, std::size_t screen) {
    if (screen >= screen_names.size()) throw std::out_of_range("Screen index");
    const auto source_pixels = archive.entry(screen);
    Bytes pixels(source_pixels.size());
    // The preserved opaque-copy routine reads its source without modifying it.
    straightcopy(static_cast<int>(source_pixels.size()), pixels.data(), const_cast<BYTE*>(source_pixels.data()));
    const auto original_palette = archive.entry(16+screen);
    const auto mouse_palette = archive.entry(15);
    if (pixels.size()!=640*480 || original_palette.size()!=768 || mouse_palette.size()!=48)
        throw std::runtime_error("Unexpected screen or palette dimensions");
    Bytes palette(original_palette.begin(),original_palette.end());
    // DisplayBackground() in EURO_GRF.CPP replaces the first 16 colors.
    std::copy(mouse_palette.begin(),mouse_palette.end(),palette.begin());
    for (auto channel : palette) if (channel > 63)
        throw std::runtime_error("Palette contains a value outside the VGA 6-bit range");
    Bytes rgba(pixels.size()*4);
    for (std::size_t i=0;i<pixels.size();++i) {
        const auto p=std::size_t(pixels[i])*3;
        for (std::size_t c=0;c<3;++c) rgba[i*4+c]=std::uint8_t(palette[p+c]*255/63);
        rgba[i*4+3]=255;
    }
    return rgba;
}
Tactics decode_tactics(std::span<const std::uint8_t> bytes) {
    if (bytes.size()!=70*10*2*4) throw std::runtime_error("Tactics table must be exactly 5600 bytes");
    Tactics result{};
    for (std::size_t z=0;z<70;++z) for (std::size_t p=0;p<10;++p) {
        const auto i=(z*10+p)*8;
        result[z][p]={std::bit_cast<std::int32_t>(u32(bytes,i)),
                      std::bit_cast<std::int32_t>(u32(bytes,i+4))};
    }
    return result;
}
}

