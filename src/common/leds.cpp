#include <Arduino.h>

#include "leds.h"
#include "tuning.h"
#include "platform.h"

namespace fk {

enum class AnimationType {
    Off,
    Static,
    Blink,
    Fade,
    Wheel
};

enum class Priority : uint8_t {
    Lowest = 0,
    Normal = 0,
    Button = 10,
    Highest = 255
};

static inline constexpr uint32_t get_color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

template<typename T>
struct Color {
    T r{ 0 };
    T g{ 0 };
    T b{ 0 };

    Color() {
    }

    Color(T r, T g, T b) : r(r), g(g), b(b) {
    }

    Color(uint32_t c) {
        r = (c >> 16) & 0xff;
        g = (c >> 8) & 0xff;
        b = (c) & 0xff;
    }

    explicit operator uint32_t() {
        return get_color(r, g, b);
    }

    Color operator * (uint32_t scalar) {
        return { r * scalar, g * scalar, b * scalar };
    }

    Color operator / (uint32_t scalar) {
        return { r / scalar, g / scalar, b / scalar };
    }
};

class LedAnimation {
private:
    AnimationType type_{ AnimationType::Off };
    Priority priority_{ Priority::Lowest };
    uint32_t color_{ 0 };
    uint16_t period_{ 0 };
    uint16_t duration_{ 0 };
    uint32_t started_{ 0 };
    uint32_t active_{ 0 };

public:
    LedAnimation() {
    }

    LedAnimation(AnimationType type, Priority priority, uint32_t color, uint16_t period, uint16_t duration) :
        type_(type), priority_(priority), color_(color), period_(period), duration_(duration) {
        started_ = fk_uptime();
    }

public:
    bool off() const {
        return type_ == AnimationType::Off;
    }

    bool black() const {
        return type_ == AnimationType::Static && color_ == 0;
    }

    Priority priority() const {
        return priority_;
    }

public:
    uint32_t update() {
        auto elapsed = (fk_uptime() - started_);

        if (duration_ > 0 && elapsed > duration_) {
            type_ = AnimationType::Off;
            return 0;
        }

        switch (type_) {
        case AnimationType::Static: {
            return color_;
        }
        case AnimationType::Wheel: {
            auto factor = (elapsed % period_) * 255 / period_;
            auto position = 255 - (factor % 255);
            if (position < 85) {
                return get_color(255 - position * 3, 0, position * 3);
            }
            else if (position < 170) {
                position -= 85;
                return get_color(0, position * 3, 255 - position * 3);
            }
            else {
                position -= 170;
                return get_color(position * 3, 255 - position * 3, 0);
            }
            break;
        }
        case AnimationType::Blink: {
            if ((elapsed % period_) < period_ / 2) {
                return color_;
            }
            break;
        }
        case AnimationType::Fade: {
            auto factor = elapsed % period_;
            auto half = period_ / 2;
            if (factor > (uint32_t)half) {
                factor = period_ - factor;
            }
            auto c = Color<uint32_t>(color_) * factor / half;
            return (uint32_t)c;
        }
        case AnimationType::Off:
        default:
            break;
        }

        return 0;
    }
};

static LedAnimation active_;

static void pushAnimation(LedAnimation incoming) {
    if (active_.off() || active_.black() || active_.priority() <= incoming.priority()) {
        active_ = incoming;
    }
}

Leds::Leds() {
}

void Leds::setup() {
    // Note that at least one board right now uses 13 for other purposes so
    // ths should be done before that happens.
    #if !defined(FK_HARDWARE_WIRE11AND13_ENABLE)
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    #endif
    // I removed the above always happening because of a change that occured in the Arduino Core.
    // Basically the call to pinMode no longer disables pullups or something (I
    // need to read up more) and so this was breaking the use of pin 13 for I2C.
    // https://github.com/arduino/ArduinoCore-samd/commit/33efce53f509e276f8c7e727ab425ed7427e9bfd

    pinMode(A3, OUTPUT);

    pixel_.begin();
    pixel_.setPixelColor(0, 0);
    pixel_.show();
}

bool Leds::task() {
    if (!active_.off()) {
        auto color = active_.update();
        pixel_.setBrightness(32);
        pixel_.setPixelColor(0, color);
        pixel_.show();
        return true;
    }
    return false;
}

bool Leds::disabled() {
    if (LedsDisableAfter == 0) {
        return false;
    }
    return fk_uptime() > LedsDisableAfter;
}

void Leds::notifyAlive() {
    pushAnimation(LedAnimation{ AnimationType::Fade, Priority::Normal, get_color(0, 0, 255), 500, 500 });
}

void Leds::notifyBattery(float percentage) {
}

void Leds::notifyNoModules() {
    pushAnimation(LedAnimation{ });
}

void Leds::notifyReadingsBegin() {
    pushAnimation(LedAnimation{ AnimationType::Static, Priority::Normal, get_color(0, 32, 0), 0, 0 });
}

void Leds::notifyReadingsDone() {
    pushAnimation(LedAnimation{ AnimationType::Static, Priority::Normal, 0, 0, 0 });
}

void Leds::notifyFatal() {
    pushAnimation(LedAnimation{ AnimationType::Blink, Priority::Highest, get_color(255, 0, 0), 500, 0 });
}

void Leds::notifyHappy() {
    pushAnimation(LedAnimation{ AnimationType::Wheel, Priority::Normal, 0, 5000, 0 });
}

void Leds::notifyButtonPressed() {
    pushAnimation(LedAnimation{ AnimationType::Static, Priority::Button, get_color(0, 16, 16), 0, 0 });
}

void Leds::notifyButtonLong() {
    pushAnimation(LedAnimation{ AnimationType::Static, Priority::Button, get_color(64, 64, 64), 0, 0 });
}

void Leds::notifyButtonShort() {
    pushAnimation(LedAnimation{ AnimationType::Static, Priority::Button, get_color(0, 64, 64), 0, 0 });
}

void Leds::notifyButtonReleased() {
    pushAnimation(LedAnimation{ AnimationType::Static, Priority::Button, 0, 0, 0 });
}

}
