#pragma once

#include <cmath>

namespace geo {
    
const double EarthRadius = 6371000;
    
struct Coordinates {
    double lat;
    double lng;
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    const double dr = M_PI / 180.0;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * EarthRadius;
}

} // конец namespace geo