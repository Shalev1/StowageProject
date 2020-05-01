#include "Spot.h"

Spot::Spot(int x, int y, bool available, int floor_num) {
    setPlace(x, y, floor_num);
    setAvailable(available);
    setContainer(nullptr);
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