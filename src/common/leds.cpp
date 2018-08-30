#include <Arduino.h>

#include "leds.h"
#include "tuning.h"
#include "platform.h"

namespace fk {

enum class AnimationType {
    Off,
    Blink,
    Fade,
    Wheel
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
    uint32_t color_{ 0 };
    uint16_t period_{ 0 };
    uint16_t duration_{ 0 };
    uint32_t started_{ 0 };
    uint32_t active_{ 0 };

public:
    LedAnimation() {
    }

    LedAnimation(AnimationType type, uint32_t color, uint16_t period, uint16_t duration) :
        type_(type), color_(color), period_(period), duration_(duration) {
        started_ = fk_uptime();
    }

public:
    bool off() {
        return type_ == AnimationType::Off;
    }

public:
    uint32_t update() {
        auto elapsed = (fk_uptime() - started_);

        if (duration_ > 0 && elapsed > duration_) {
            type_ = AnimationType::Off;
            return 0;
        }

        switch (type_) {
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

void Leds::task() {
    if (!active_.off()) {
        auto color = active_.update();
        pixel_.setBrightness(32);
        pixel_.setPixelColor(0, color);
        pixel_.show();
    }
}

bool Leds::disabled() {
    if (LedsDisableAfter == 0) {
        return false;
    }
    return fk_uptime() > LedsDisableAfter;
}

void Leds::notifyAlive() {
    active_ = LedAnimation{ AnimationType::Fade, get_color(0, 0, 255), 500, 500 };
}

void Leds::notifyBattery(float percentage) {
}

void Leds::notifyNoModules() {
    pixel_.setPixelColor(0, 0, 0, 0);
    pixel_.show();
}

void Leds::notifyReadingsBegin() {
    pixel_.setPixelColor(0, 0, 32, 0);
    pixel_.show();
}

void Leds::notifyReadingsDone() {
    pixel_.setPixelColor(0, 0, 0, 0);
    pixel_.show();
}

void Leds::notifyFatal() {
    active_ = LedAnimation{ AnimationType::Blink, get_color(255, 0, 0), 500, 0 };
}

void Leds::notifyHappy() {
    active_ = LedAnimation{ AnimationType::Wheel, 0, 5000, 0 };
}

}
