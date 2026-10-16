#pragma once
#include <filesystem>
namespace actua {
struct MatchOptions {
    int wind=0;
    bool offside=true, fouls=true, bookings=true;
    int referee_accuracy=96, referee_strictness=96;
    bool valid() const;
    bool operator==(const MatchOptions&) const = default;
};
struct MatchPreferences {
    MatchOptions match;
    int music=100, effects=100, crowd=100, commentary=75;
    int commentary_language=0; // 0 English, 1 French; original recordings only.
    bool muted=false;
    bool valid() const;
    void save(const std::filesystem::path&) const;
    void load(const std::filesystem::path&);
    bool operator==(const MatchPreferences&) const = default;
};
}
