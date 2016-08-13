#include "luisavm.hh"

#include <exception>
#include <iostream>
using namespace std;

// {{{ test infrastructure

struct test_failed : exception {
    const char* what() const noexcept override { return "test failed"; }
};

template<typename T, typename U>
void _equals(T&& tested, U&& expected, string const& msg1="", string const& msg2="")
{
    string msg = (msg2 != "") ? msg2 : msg1;

    if(tested == expected) {
        cout << "[\e[32mok\e[0m] " << msg << " == 0x" << hex << uppercase 
             << static_cast<int>(tested) << endl;
    } else {
        cout << "[\e[31merr\e[0m] " << msg << " == 0x" << hex << uppercase 
             << static_cast<int>(tested) << " (expected 0x" << expected << ")" << endl;
        throw test_failed();
    }
}

#define equals(test, expect, ...) \
    (_equals(test, expect, #test, ##__VA_ARGS__))

// }}}

static void luisavm_tests();
static void cpu_tests();

void run_tests()
{
    luisavm_tests();
    cpu_tests();
}

// {{{ luisavm_tests

static void luisavm_tests()
{
    cout << "#\n";
    cout << "# luisavm\n";
    cout << "#\n";

    luisavm::LuisaVM c;
    c.Set(0x12, 0xAF);
    equals(c.Get(0x12), 0xAF, "memory set");

    c.Set32(0x0, 0x12345678);
    equals(c.Get(0x0), 0x78, "set32 (byte 0)");
    equals(c.Get(0x1), 0x56, "set32 (byte 1)");
    equals(c.Get(0x2), 0x34, "set32 (byte 2)");
    equals(c.Get(0x3), 0x12, "set32 (byte 3)");

    equals(c.PhysicalMemory()[0x3], 0x12);

    // TODO - offset tests
}

// }}}

// {{{ cpu_tests

static void cpu_santiy();


static void cpu_tests()
{
    cout << "#\n";
    cout << "# cpu\n";
    cout << "#\n";

    cpu_santiy();
}


static void cpu_santiy()
{
    luisavm::LuisaVM c;

    cout << "# sanity\n";
    luisavm::CPU& cpu = c.cpu();

    cpu.A = 0x24;
    cpu.B = 0xBF;
    equals(cpu.B, 0xBF);
    equals(cpu.A, 0x24);

    equals(cpu.Flag(luisavm::Flag::Z), false);
    cpu.setFlag(luisavm::Flag::Z, true);
    equals(cpu.Flag(luisavm::Flag::Z), true);
    equals(cpu.FL, 0x4);
}

// }}}
