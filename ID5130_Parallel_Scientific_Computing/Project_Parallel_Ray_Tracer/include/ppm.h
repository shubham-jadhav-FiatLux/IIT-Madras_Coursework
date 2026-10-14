#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace rt {

inline bool write_ppm(const std::string& path, int width, int height,
                      const std::vector<uint8_t>& pixels) {
    if (pixels.size() != static_cast<size_t>(width) * height * 3) return false;
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out << "P6\n" << width << " " << height << "\n255\n";
    out.write(reinterpret_cast<const char*>(pixels.data()),
              static_cast<std::streamsize>(pixels.size()));
    return static_cast<bool>(out);
}

} // namespace rt
