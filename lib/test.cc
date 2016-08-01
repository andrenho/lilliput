#include "luisavm.hh"
#include "assembler.hh"

#include <exception>
#include <iostream>
using namespace std;

namespace luisavm {

// {{{ test infrastructure

struct test_failed : exception {
    const char* what() const noexcept override { return "test failed"; }
};

template<typename T, typename U>
void _equals(T&& tested, U&& expected, string const& msg1="", string const& msg2="")
{
    string msg = (msg2 != "") ? msg2 : msg1;

    if(static_cast<ssize_t>(tested) == static_cast<ssize_t>(expected)) {
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

void test_assembler(string const& name, string const& code, vector<uint8_t> const& expected)
{
    vector<uint8_t> result = Assembler().AssembleString(name, code);
    if(result == expected) {
        cout << "[\e[32mok\e[0m] " << name << endl;
    } else {
        cout << "[\e[31merr\e[0m] " << name << endl;
        cout << "Expected: "; 
        for(auto v: expected) {
            cout << hex << uppercase << "0x" << static_cast<int>(v) << ", ";
        }
        cout << "\n";
        cout << "Result: "; 
        for(auto v: result) {
            cout << hex << uppercase << "0x" << static_cast<int>(v) << ", ";
        }
        cout << "\n";
        throw test_failed();
    }
}

void test_assembler_map(string const& name, string const& code, string const& expected)
{
    string result;
    Assembler().AssembleString(name, code, result);
    if(result == expected) {
        cout << "[\e[32mok\e[0m] " << name << endl;
    } else {
        cout << "[\e[31merr\e[0m] " << name << endl;
        cout << "Expected: \n" + expected + "\n";
        cout << "Result: \n" + result + "\n";
        throw test_failed();
    }
}

template<typename F, typename U>
luisavm::LuisaVM _code(function<void(luisavm::LuisaVM& c, luisavm::CPU&)> pre, string const& code, F&& test, U&& expected)
{
    luisavm::LuisaVM comp;
    pre(comp, comp.cpu());

    luisavm::Assembler().AssembleString("test", "section .text\n" + code);
    luisavm::CPU& cpu = comp.cpu();
    auto tested = test(comp, cpu);
    equals(tested, expected, code);

    return comp;
}

#define code(pre, code, tested, expected) \
    (_code([](luisavm::LuisaVM& comp, luisavm::CPU& cpu) { (void)comp; (void)cpu; pre; }, code, [](luisavm::LuisaVM const& comp, luisavm::CPU const& cpu) { (void) comp; (void) cpu; return tested; }, expected))

// }}}

static void luisavm_tests();
static void assembler_tests();
static void cpu_tests();

void run_tests()
{
    luisavm_tests();
    assembler_tests();
    cpu_tests();
}

// {{{ luisavm_tests

static void luisavm_tests()
{
    cout << "#\n";
    cout << "# luisavm\n";
    cout << "#\n";

    LuisaVM c;
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

// {{{ assembler_tests

static void assembler_tests()
{
    typedef vector<uint8_t> V;

    cout << "#\n";
    cout << "# assembler\n";
    cout << "#\n";

    test_assembler("basic test", R"(
    section .text
    mov D, 0x64   ; comment)", V { 0x2, 0x3, 0x64 });

    test_assembler("two commands", R"(
    section .text
    mov B, C )", V { 0x1, 0x12 });

    test_assembler("data", R"(
    section .text
    mov A, B
    section .data
    .db 0x12, 0x34 )", V { 0x1, 0x1, 0x12, 0x34 });

    test_assembler("data + string", R"(
    section .text
    mov A, B
    section .data
    .dw 0x1234
    .ascii abc )", V { 0x1, 0x1, 0x34, 0x12, 0x61, 0x62, 0x63 });

    test_assembler("data + string (inverted order)", R"(
    section .data
    .dw 0x1234
    section .text
    mov A, B
    section .data
    .asciiz abc )", V { 0x1, 0x1, 0x34, 0x12, 0x61, 0x62, 0x63, 0x0 });

    test_assembler("res", R"(
    section .text
    mov A, B
    section .bss
    .resb 8 )", V { 0x1, 0x1 });

    test_assembler("constants", R"(
%define TEST 0x1234
    section .text
    mov A, TEST )", V { 0x3, 0x0, 0x34, 0x12 });

    test_assembler("label", R"(
    section .text
    jmp test
    test: mov A, B )", V { 0x65, 0x5, 0x0, 0x0, 0x0, 0x1, 0x1 });

    test_assembler("local label", R"(
    section .text
    a: jmp .test
    .test: mov A, B
    b: jmp .test
    .test: mov A, B )", V { 0x65, 5, 0, 0, 0, 0x1, 0x1,
                            0x65, 12, 0, 0, 0, 0x1, 0x1 });

    test_assembler("label on data", R"(
    section .text
    jmp test
    section .data
    test: .db 0 )", V { 0x65, 5, 0, 0, 0, 0 });

    test_assembler("label on bss", R"(
    section .text
    jmp test
    section .bss
    .resb 4
    test: .resb 4 )", V { 0x65, 9, 0, 0, 0 });

    test_assembler("include", R"(%import data/test.s)", 
            V { 0x1, 0x1 });

    test_assembler_map("map", R"(section .text

            mov     C, 10   ; initialize C
    next:   dec     C
            cmp     C, 0    ; TODO - this is not needed
            bnz     next

    halt:   jmp     halt)", R"(0:map
**
0:3:0
0:4:3
0:5:5
0:6:8
0:8:13
)");
}

// }}}

// {{{ cpu_tests

static void cpu_santiy();
static void mov();
static void movb();


static void cpu_tests()
{
    cout << "#\n";
    cout << "# cpu\n";
    cout << "#\n";

    cpu_santiy();
    mov();
    movb();
}


static void cpu_santiy()
{
    LuisaVM c;

    cout << "# sanity\n";
    CPU& cpu = c.cpu();

    cpu.A = 0x24;
    cpu.B = 0xBF;
    equals(cpu.B, 0xBF);
    equals(cpu.A, 0x24);

    equals(cpu.Flag(Flag::Z), false);
    cpu.setFlag(Flag::Z, true);
    equals(cpu.Flag(Flag::Z), true);
    equals(cpu.FL, 0x4);
}


static void mov()
{
    cout << "# mov\n";

    LuisaVM c = code(cpu.B = 0x42, "mov A, B", cpu.A, 0x42);
    equals(c.cpu().PC, 2);

    code({}, "mov A, 0x34", cpu.A, 0x34);
    code({}, "mov A, 0x1234", cpu.A, 0x1234);
    code({}, "mov A, 0xFABC1234", cpu.A, 0xFABC1234);

    cout << "# movement flags\n";
    c = code({}, "mov A, 0", cpu.Flag(Flag::Z), true);
    equals(c.cpu().Flag(Flag::S), false);

    c = code({}, "mov A, 0xF0000001", cpu.Flag(Flag::Z), false);
    equals(c.cpu().Flag(Flag::S), true);
}


static void movb()
{
    cout << "# movb\n";

    code({ cpu.B = 0x1000; comp.Set(cpu.B, 0xAB); }, "movb A, [B]", cpu.A, 0xAB);
    code({ comp.Set(0x1000, 0xAB) ; }, "movb A, [0x1000]", cpu.A, 0xAB);
    code({ cpu.A = 0x64 ; cpu.C = 0x32 ; }, "movb [C], A", comp.Get(0x32), 0x64);
    code({ cpu.A = 0x64 ; }, "movb [A], 0xFA", comp.Get(0x64), 0xFA);
    code({ cpu.A = 0x32 ; cpu.B = 0x64 ; comp.Set(0x64, 0xFF) ; }, "movb [A], [B]", comp.Get(0x32), 0xFF);
    code({ cpu.A = 0x32; comp.Set(0x6420, 0xFF) ; }, "movb [A], [0x6420]", comp.Get(0x32), 0xFF);
    code({ cpu.A = 0xAC32 ; }, "movb [0x64], A", comp.Get(0x64), 0x32);
    code({}, "movb [0x64], 0xF0", comp.Get(0x64), 0xF0);
    code({ cpu.A = 0xF000 ; comp.Set(0xF000, 0x42); }, "movb [0xCC64], [A]", comp.Get(0xCC64), 0x42);
    code({ comp.Set32(0xABF0, 0x3F); }, "movb [0x64], [0xABF0]", comp.Get(0x64), 0x3F);
}


// }}}

}  // namespace luisavm
