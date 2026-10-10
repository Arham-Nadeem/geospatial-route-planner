\
#pragma once

#include <algorithm>
#include <cmath>

struct Coordinate {
    double latitude{};
    double longitude{};
};

inline double degToRad(double degrees) {
    constexpr double pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
}

inline double haversineMiles(const Coordinate& a, const Coordinate& b) {
    //Compute straight-line surface distance between two latitude/longitude points
    constexpr double earthRadiusMiles = 3958.7613;

    const double lat1 = degToRad(a.latitude);
    const double lat2 = degToRad(b.latitude);
    const double dLat = degToRad(b.latitude - a.latitude);
    const double dLon = degToRad(b.longitude - a.longitude);

    const double sinLat = std::sin(dLat / 2.0);
    const double sinLon = std::sin(dLon / 2.0);

    const double h = sinLat * sinLat +
                     std::cos(lat1) * std::cos(lat2) * sinLon * sinLon;

    const double clamped = std::clamp(h, 0.0, 1.0);
    const double arc = 2.0 * std::asin(std::sqrt(clamped));
    return earthRadiusMiles * arc;
}
