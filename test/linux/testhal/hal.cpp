#include <gtest/gtest.h>
#include <Arduino.h>

Uart Serial;

void __fk_assert(const char *msg, const char *file, int lineno) {
    ADD_FAILURE_AT(file, lineno) << msg;
}
