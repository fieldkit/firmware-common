#include "debug.h"

namespace std {

void __throw_bad_alloc() {
    debugfln("std::bad_alloc");
    while (true) {
    }
}

}
