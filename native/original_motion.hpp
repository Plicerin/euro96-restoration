#pragma once
#include <array>
#include <vector>
#include <string>
namespace actua {
struct OriginalClip {int record,animation=-1;std::string name;std::vector<std::array<float,85>> frames;};
}
