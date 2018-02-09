#ifndef FK_RESTART_WIZARD_H_INCLUDED
#define FK_RESTART_WIZARD_H_INCLUDED

#include "debug.h"

namespace fk {

struct Call {
    void *function;
    void *site;
};

template<typename T>
class History {
private:
    static constexpr size_t Size = 16;
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
        /*
        auto previous = head == 0 ? history[Size - 1] : history[head - 1];
        if (previous == entry) {
            return;
        }
        */
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

};

class RestartWizard {
private:
    History<const char*> history;
    History<Call> calls;

public:
    void startup();
    void checkin(const char *where);
    void checkin(Call call);
    void dump();

};

extern RestartWizard restartWizard;

}

#endif
