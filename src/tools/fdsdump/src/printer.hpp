#pragma once

#include <cstdint>

class Printer
{
public:
    virtual
    ~Printer() {}

    virtual void
    print_prologue() = 0;

    virtual void
    print_record(uint8_t *record) = 0;

    virtual void
    print_epilogue() = 0;

};