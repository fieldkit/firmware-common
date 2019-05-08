#include <Arduino.h>

#include <alogging/alogging.h>

#include "leds.h"
#include "tuning.h"
#include "platform.h"
#include "configuration.h"

namespace fk {

constexpr const char Log[] = "LEDs";

using Logger = SimpleLog<Log>;

enum class AnimationType {
    None,
    Done,
    Static,
    Blink,
    Fade,
    Wheel
};

static const char *animation_type_name(AnimationType a) {
    switch (a) {
    case AnimationType::None: return "None";
    case AnimationType::Done: return "Done";
    case AnimationType::Static: return "Static";
    case AnimationType::Blink: return "Blink";
    case AnimationType::Fade: return "Fade";
    case AnimationType::Wheel: return "Wheel";
    default: return "Unknown";
    }
}

enum class Priority : uint8_t {
    Lowest = 0,
    Normal = 1,
    Wifi = 20,
    Alive = 25,
    Readings = 30,
    Button = 200,
    Highest = 255
};

static const char *priority_name(Priority p) {
    switch (p) {
    case Priority::Lowest: return "Lowest";
    case Priority::Normal: return "Normal";
    case Priority::Alive: return "Alive";
    case Priority::Wifi: return "Wifi";
    case Priority::Readings: return "Readings";
    case Priority::Button: return "Button";
    case Priority::Highest: return "Highest";
    default: return "Unknown";
    }
}

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
    AnimationType type_{ AnimationType::None };
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
    bool none() const {
        return type_ == AnimationType::None;
    }

    bool done() const {
        return type_ == AnimationType::Done;
    }

    bool black() const {
        return type_ == AnimationType::Static && color_ == 0;
    }

    Priority priority() const {
        return priority_;
    }

    AnimationType type() const {
        return type_;
    }

public:
    uint32_t update() {
        auto elapsed = (fk_uptime() - started_);

        if (duration_ > 0 && elapsed > duration_) {
            type_ = AnimationType::Done;
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
        default:
            break;
        }

        return 0;
    }
};

static constexpr size_t ActiveSize = 3;
static LedAnimation active_[ActiveSize];

static size_t get_available(Priority priority) {
    for (auto i = (size_t)0; i < ActiveSize; ++i) {
        if (active_[i].none()) {
            return i;
        }
    }
    for (auto i = (size_t)0; i < ActiveSize; ++i) {
        if (active_[i].priority() < priority) {
            return i;
        }
    }
    return ActiveSize;
}

static void pushAnimation(bool disabled, LedAnimation incoming) {
    if (disabled) {
        return;
    }

    auto available = get_available(incoming.priority());
    if (available >= ActiveSize) {
        Logger::trace("No available slots: %s", priority_name(incoming.priority()));
        for (auto i = (size_t)0; i < ActiveSize; ++i) {
            Logger::trace("%d: %s %s", i, priority_name(active_[i].priority()), animation_type_name(active_[i].type()));
        }
        return;
    }

    Logger::trace("%d: New State: %s", available, priority_name(incoming.priority()));

    active_[available] = incoming;
}

static void cancel(Priority priority) {
    for (auto i = (size_t)0; i < ActiveSize; ++i) {
        if (active_[i].priority() == priority) {
            Logger::trace("%d: Cancel: %s", i, priority_name(priority));
            active_[i] = { };
            for (auto j = i; j < ActiveSize; ++j) {
                active_[i] = active_[j];
            }
        }
    }
}

Leds::Leds() {
}

void Leds::setup() {
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    // I removed the above always happening because of a change that occured in the Arduino Core.
    // Basically the call to pinMode no longer disables pullups or something (I
    // need to read up more) and so this was breaking the use of pin 13 for I2C.
    // https://github.com/arduino/ArduinoCore-samd/commit/33efce53f509e276f8c7e727ab425ed7427e9bfd

    pinMode(A3, OUTPUT);

    pixel_.begin();
    if (configuration.common.leds.brightness > 0) {
        pixel_.setBrightness(configuration.common.leds.brightness);
    }
    pixel_.setPixelColor(0, get_color(8, 8, 8));
    pixel_.show();
}

bool Leds::task() {
    Priority selected = Priority::Lowest;
    uint32_t color = 0;
    for (auto &la : active_) {
        if (la.none()) {
            continue;
        }
        if (la.done()) {
            la = { }; // Set to None.
            continue;
        }
        if (la.priority() > selected) {
            color = la.update();
            selected = la.priority();
        }
    }
    if (color > 0 || pixel_.getPixelColor(0) > 0) {
        if (configuration.common.leds.brightness > 0) {
            pixel_.setBrightness(configuration.common.leds.brightness);
        }
        pixel_.setPixelColor(0, color);
        pixel_.show();
    }
    return color > 0;
}

bool Leds::disabled() {
    if (configuration.common.leds.disable_after == 0) {
        return false;
    }
    return (fk_uptime() - user_activity_) > configuration.common.leds.disable_after;
}

void Leds::off() {
    Logger::trace("OFF");
    for (auto &la: active_) {
        la = LedAnimation{ };
    }
    pixel_.setPixelColor(0, 0);
    pixel_.show();
}

void Leds::notifyInitialized() {
    Logger::trace("Initialized");
    pixel_.setPixelColor(0, get_color(16, 16, 16));
    pixel_.show();
}

void Leds::notifyStarted() {
    pixel_.setPixelColor(0, 0);
    pixel_.show();
}

void Leds::notifyAlive() {
    pushAnimation(disabled(), LedAnimation{ AnimationType::Fade, Priority::Alive, get_color(0, 0, 255), 500, 500 });
}

void Leds::notifyBattery(float percentage) {
}

void Leds::notifyNoModules() {
    pushAnimation(disabled(), LedAnimation{ });
}

void Leds::notifyReadingsBegin() {
    Logger::trace("ReadingsBegin");
    pushAnimation(disabled(), LedAnimation{ AnimationType::Static, Priority::Readings, get_color(255, 175, 10), 0, 0 });
}

void Leds::notifyReadingsDone() {
    cancel(Priority::Readings);
}

void Leds::notifyCaution() {
    pushAnimation(disabled(), LedAnimation{ AnimationType::Blink, Priority::Highest, get_color(255, 165, 0), 500, 0 });
}

void Leds::notifyWarning() {
    pushAnimation(disabled(), LedAnimation{ AnimationType::Blink, Priority::Highest, get_color(255, 255, 0), 500, 0 });
}

void Leds::notifyFatal() {
    pushAnimation(disabled(), LedAnimation{ AnimationType::Blink, Priority::Highest, get_color(255, 0, 0), 500, 0 });
}

void Leds::notifyHappy() {
    pushAnimation(disabled(), LedAnimation{ AnimationType::Wheel, Priority::Normal, 0, 5000, 0 });
}

void Leds::notifyButtonPressed() {
    Logger::trace("ButtonPressed");
    user_activity_ = fk_uptime();
    pushAnimation(disabled(), LedAnimation{ AnimationType::Static, Priority::Button, get_color(0, 16, 16), 0, 0 });
}

void Leds::notifyTopPassed() {
    Logger::trace("TopPassed");
    pushAnimation(disabled(), LedAnimation{ AnimationType::Static, Priority::Normal, get_color(0, 0, 255), 0, 0 });
}

void Leds::notifyButtonLong() {
    Logger::trace("ButtonLong");
    pushAnimation(disabled(), LedAnimation{ AnimationType::Static, Priority::Button, get_color(255, 255, 255), 0, 0 });
}

void Leds::notifyButtonShort() {
    Logger::trace("ButtonShort");
    pushAnimation(disabled(), LedAnimation{ AnimationType::Static, Priority::Button, get_color(0, 64, 64), 0, 0 });
}

void Leds::notifyButtonReleased() {
    cancel(Priority::Button);
}

void Leds::notifyWifiOn() {
    pushAnimation(disabled(), LedAnimation{ AnimationType::Static, Priority::Wifi, get_color(233, 51, 255), 0, 0 });
}

void Leds::notifyWifiOff() {
    cancel(Priority::Wifi);
}

}
