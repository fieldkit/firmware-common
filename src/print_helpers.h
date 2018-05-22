#ifndef FK_PRINT_HELPERS_H_INCLUDED
#define FK_PRINT_HELPERS_H_INCLUDED

namespace fk {

class PrintSizeCalculator : public Print {
private:
    size_t size{ 0 };

public:
    size_t getSize() {
        return size;
    }

    size_t write(uint8_t c) override {
        size++;
        return 1;
    }
};

class BufferPrinter : public Print {
public:
    char *buffer;
    size_t size;
    size_t pos{ 0 };

public:
    BufferPrinter(char *buffer, size_t size) : buffer(buffer), size(size) {
    }

    size_t write(uint8_t c) override {
        if (pos < size) {
            buffer[pos++] = c;
            buffer[pos] = 0;
            return 1;
        }
        return 0;
    }

};

}

#endif
