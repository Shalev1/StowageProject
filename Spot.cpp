#include "Spot.h"

Spot::Spot(int x, int y, bool available, int floor_num) {
    setPlace(x, y, floor_num);
    setAvailable(available);
    setContainer(nullptr);
}

Spot::Spot(const Spot &s) {
    setPlace(s.place.first, s.place.second, s.floor_num);
    setAvailable(s.available);
    setContainer(s.cont);
}

Spot &Spot::operator=(const Spot &other) {
    if (this != &other) {
        setPlace(other.place.first, other.place.second, other.floor_num);
        setAvailable(other.available);
        setContainer(other.cont);
    }
    return *this;
}

ostream &operator<<(ostream &out, const Spot &s) {
    out << "Spot details are:" << endl;
    if (s.cont != nullptr) {
        return out << "Location: (" << s.place.first << "," << s.place.second << "), "
                   << "Floor: " << s.floor_num << ", "
                   << "Available: " << s.available << ",\n"
                   << "Container: " << *(s.cont)
                   << endl;
    } else {
        return out << "Location: (" << s.place.first << "," << s.place.second << "), "
                   << "Floor: " << s.floor_num << ", "
                   << "Available: " << s.available << ",\n"
                   << "Container: No Container"
                   << endl;
    }
}