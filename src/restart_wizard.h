#ifndef FK_RESTART_WIZARD_H_INCLUDED
#define FK_RESTART_WIZARD_H_INCLUDED

#include "debug.h"

namespace fk {

struct Call {
    void *function;
    void *site;
    bool returned : 1;
};

template<typename T>
class History {
private:
    static constexpr size_t Size = 48;
    T history[Size];
    size_t head;

public:
    void startup() {
        head = 0;
        for (size_t i = 0; i < Size; ++i) {
            history[i] = T{};
        }
    }

    void checkin(T entry) {
        history[head] = entry;
        head++;
        if (head == Size) {
            head = 0;
        }
    }

    template<typename Func>
    void dump(Func dumpFunc) {
        auto i = (int32_t)head - 1;
        do {
            if (0 > i) {
                i = Size - 1;
            }
            dumpFunc(i, history[i]);
            if (i == (int32_t)head) break;
            i--;
        }
        while (true);
    }

    T &last() {
        return head == 0 ? history[Size - 1] : history[head - 1];
    }

};

class RestartWizard {
private:
    History<Call> calls;
    uint32_t lastLoop;
    uint32_t deepestStack;

public:
    void startup();
    void checkin(Call call);
    void looped() {
        lastLoop = millis();
    }
    void dump();
    Call &lastCall() {
        return calls.last();
    }

};

extern RestartWizard restartWizard;

}

#endif
