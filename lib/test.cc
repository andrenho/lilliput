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
    string bin = Assembler().AssembleString(name, code);
    vector<uint8_t> result;  // TODO
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

template<typename F, typename U>
LuisaVM _code(function<void(LuisaVM& c, CPU&)> pre, string const& code, F&& test, U&& expected)
{
    LuisaVM comp(1024*1024);
    pre(comp, comp.cpu());

    vector<uint8_t> data; // TODO = Assembler().AssembleString("test", "section .text\n" + code);
    for(uint32_t i=0; i<data.size(); ++i) {
        comp.Set(i, data[i]);
    }
    comp.Step();
    CPU& cpu = comp.cpu();
    auto tested = test(comp, cpu);
    equals(tested, expected, code);

    return comp;
}

#define code(pre, code, tested, expected) \
    (_code([](LuisaVM& comp, CPU& cpu) { (void)comp; (void)cpu; pre; }, code, [](LuisaVM const& comp, CPU const& cpu) { (void) comp; (void) cpu; return tested; }, expected))

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

    /*
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
    */
}

// }}}

// {{{ cpu_tests

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
    code({ comp.Set(0x1000, 0xAB); }, "movb A, [0x1000]", cpu.A, 0xAB);
    code({ cpu.A = 0x64; cpu.C = 0x32; }, "movb [C], A", comp.Get(0x32), 0x64);
    code({ cpu.A = 0x64; }, "movb [A], 0xFA", comp.Get(0x64), 0xFA);
    code({ cpu.A = 0x32; cpu.B = 0x64; comp.Set(0x64, 0xFF); }, "movb [A], [B]", comp.Get(0x32), 0xFF);
    code({ cpu.A = 0x32; comp.Set(0x6420, 0xFF); }, "movb [A], [0x6420]", comp.Get(0x32), 0xFF);
    code({ cpu.A = 0xAC32; }, "movb [0x64], A", comp.Get(0x64), 0x32);
    code({}, "movb [0x64], 0xF0", comp.Get(0x64), 0xF0);
    code({ cpu.A = 0xF000; comp.Set(0xF000, 0x42); }, "movb [0xCC64], [A]", comp.Get(0xCC64), 0x42);
    code({ comp.Set32(0xABF0, 0x3F); }, "movb [0x64], [0xABF0]", comp.Get(0x64), 0x3F);
}


static void movw()
{
    cout << "# movw\n";

    code({ cpu.B = 0x1000; comp.Set16(cpu.B, 0xABCD); }, "movw A, [B]", cpu.A, 0xABCD);  
    code({ comp.Set16(0x1000, 0xABCD); }, "movw A, [0x1000]", cpu.A, 0xABCD);
    code({ cpu.A = 0x6402; }, "movw [A], A", comp.Get16(0x6402), 0x6402);
    code({ cpu.A = 0x64; }, "movw [A], 0xFABA", comp.Get16(0x64), 0xFABA);
    code({ cpu.A = 0x32CC; cpu.B = 0x64; comp.Set16(0x64, 0xFFAB); }, "movw [A], [B]", comp.Get16(0x32CC), 0xFFAB);
    code({ cpu.A = 0x32; comp.Set16(0x6420, 0xFFAC); }, "movw [A], [0x6420]", comp.Get16(0x32), 0xFFAC);
    code({ cpu.A = 0xAB32AC; }, "movw [0x64], A", comp.Get16(0x64), 0x32AC);
    code({}, "movw [0x64], 0xF0FA", comp.Get16(0x64), 0xF0FA);  
    code({ cpu.A = 0xF000; comp.Set16(0xF000, 0x4245); }, "movw [0xCC64], [A]", comp.Get16(0xCC64), 0x4245);  
    code({ comp.Set16(0xABF0, 0x3F54); }, "movw [0x64], [0xABF0]", comp.Get16(0x64), 0x3F54);
}


static void movd()
{
    cout << "# movw\n";

    code({ cpu.B = 0x1000; comp.Set32(cpu.B, 0xABCDEF01); }, "movd A, [B]", cpu.A, 0xABCDEF01);  
    code({ comp.Set32(0x1000, 0xABCDEF01); }, "movd A, [0x1000]", cpu.A, 0xABCDEF01);
    code({ cpu.A = 0x16402; }, "movd [A], A", comp.Get32(0x16402), 0x16402);
    code({ cpu.A = 0x64; }, "movd [A], 0xFABA1122", comp.Get32(0x64), 0xFABA1122);
    code({ cpu.A = 0x32CC; cpu.B = 0x64; comp.Set32(0x64, 0xFFAB5678); }, "movd [A], [B]", comp.Get32(0x32CC), 0xFFAB5678);
    code({ cpu.A = 0x32; comp.Set32(0x6420, 0xFFAC9876); }, "movd [A], [0x6420]", comp.Get32(0x32), 0xFFAC9876);
    code({ cpu.A = 0xAB32AC44; }, "movd [0x64], A", comp.Get32(0x64), 0xAB32AC44);
    code({}, "movd [0x64], 0xF0FA1234", comp.Get32(0x64), 0xF0FA1234);  
    code({ cpu.A = 0xF000; comp.Set32(0xF000, 0x4245AABB); }, "movd [0xCC64], [A]", comp.Get32(0xCC64), 0x4245AABB);  
    code({ comp.Set32(0xABF0, 0x3F54FABC); }, "movd [0x64], [0xABF0]", comp.Get32(0x64), 0x3F54FABC);
}


static void swap()
{
    cout << "# swap\n";
        
    LuisaVM c = code({ cpu.A = 0xA; cpu.B = 0xB; }, "swap A, B", cpu.A, 0xB);
    equals(c.cpu().B, 0xA);
}


static void logic()
{
    cout << "# logic operators\n";

    LuisaVM c = code({ cpu.A = 0b1010; cpu.B = 0b1100; }, "or A, B", cpu.A, 0b1110);        
    equals(c.cpu().Flag(Flag::S), false);
    equals(c.cpu().Flag(Flag::Z), false);
    equals(c.cpu().Flag(Flag::Y), false);
    equals(c.cpu().Flag(Flag::V), false);

    code({ cpu.A = 0b11; }, "or A, 0x4", cpu.A, 0b111);
    code({ cpu.A = 0b111; }, "or A, 0x4000", cpu.A, 0x4007);
    code({ cpu.A = 0x10800000; }, "or A, 0x2A426653", cpu.A, 0x3AC26653);
    code({ cpu.A = 0b1010; cpu.B = 0b1100; }, "xor A, B", cpu.A, 0b110);
    code({ cpu.A = 0b11; }, "xor A, 0x4", cpu.A, 0b111);
    code({ cpu.A = 0xFF0; }, "xor A, 0xFF00", cpu.A, 0xF0F0);
    code({ cpu.A = 0x148ABD12; }, "xor A, 0x2A426653", cpu.A, 0x3EC8DB41);
    c = code({ cpu.A = 0b11; cpu.B = 0b1100; }, "and A, B", cpu.A, 0);
    equals(c.cpu().Flag(Flag::Z), true);

    code({ cpu.A = 0b11; }, "and A, 0x7", cpu.A, 0b11);
    code({ cpu.A = 0xFF0; }, "and A, 0xFF00", cpu.A, 0xF00);
    code({ cpu.A = 0x148ABD12; }, "and A, 0x2A426653", cpu.A, 0x22412);
    code({ cpu.A = 0b10101010; cpu.B = 4; }, "shl A, B", cpu.A, 0b101010100000);
    code({ cpu.A = 0b10101010; }, "shl A, 4", cpu.A, 0b101010100000);
    code({ cpu.A = 0b10101010; cpu.B = 4; }, "shr A, B", cpu.A, 0b1010);
    code({ cpu.A = 0b10101010; }, "shr A, 4", cpu.A, 0b1010);
    code({ cpu.A = 0b11001010; }, "not A", cpu.A, 0xFFFFFF35);
}


static void integer_math()
{
    cout << "# integer math\n";

    code({ cpu.A = 0x12; cpu.B = 0x20; }, "add A, B", cpu.A, 0x32);  
    code({ cpu.A = 0x12; }, "add A, 0x20", cpu.A, 0x32);
    code({ cpu.A = 0x12; cpu.setFlag(Flag::Y, true); }, "add A, 0x20", cpu.A, 0x33); // with carry
    code({ cpu.A = 0x12; }, "add A, 0x2000", cpu.A, 0x2012);
    LuisaVM c = code({ cpu.A = 0x10000012; }, "add A, 0xF0000000", cpu.A, 0x12);
    equals(c.cpu().Flag(Flag::Y), true);

    c = code({ cpu.A = 0x30; cpu.B = 0x20; }, "sub A, B", cpu.A, 0x10);
    equals(c.cpu().Flag(Flag::S), false);

    c = code({ cpu.A = 0x20; cpu.B = 0x30; }, "sub A, B", cpu.A, 0xFFFFFFF0); // sub A, B (negative)
    equals(c.cpu().Flag(Flag::S), true);

    code({ cpu.A = 0x22; }, "sub A, 0x20", cpu.A, 0x2);
    code({ cpu.A = 0x22; cpu.setFlag(Flag::Y, true); }, "sub A, 0x20", cpu.A, 0x1); // sub A, 0x20 (with carry)
    c = code({ cpu.A = 0x12; }, "sub A, 0x2000", cpu.A, 0xFFFFE012);
    equals(c.cpu().Flag(Flag::S), true);
    equals(c.cpu().Flag(Flag::Y), true);

    c = code({ cpu.A = 0x10000012; }, "sub A, 0xF0000000", cpu.A, 0x20000012);
    equals(c.cpu().Flag(Flag::Y), true);

    code({}, "cmp A, B", cpu.Flag(Flag::Z), true);
    code({}, "cmp A, 0x12", cpu.Flag(Flag::LT) && !cpu.Flag(Flag::GT), true);
    code({ cpu.A = 0x6000; }, "cmp A, 0x1234", !cpu.Flag(Flag::LT) && cpu.Flag(Flag::GT), true);
    code({ cpu.A = 0xF0000000; }, "cmp A, 0x12345678", !cpu.Flag(Flag::LT) && cpu.Flag(Flag::GT), true);  // because of the signal!

    code({ cpu.A = 0x0; }, "cmp A", cpu.Flag(Flag::Z), true);
    code({ cpu.A = 0xF0; cpu.B = 0xF000; }, "mul A, B", cpu.A, 0xE10000);
    code({ cpu.A = 0x1234; }, "mul A, 0x12", cpu.A, 0x147A8);
    c = code({ cpu.A = 0x1234; }, "mul A, 0x12AF", cpu.A, 0x154198C);
    equals(c.cpu().Flag(Flag::V), false);

    c = code({ cpu.A = 0x1234; }, "mul A, 0x12AF87AB", cpu.A, 0x233194BC);
    equals(c.cpu().Flag(Flag::V), true);

    code({ cpu.A = 0xF000; cpu.B = 0xF0; }, "idiv A, B", cpu.A, 0x100);
    code({ cpu.A = 0x1234; }, "idiv A, 0x12", cpu.A, 0x102);
    code({ cpu.A = 0x1234; }, "idiv A, 0x2AF", cpu.A, 0x6);
    code({ cpu.A = 0x123487AB; }, "idiv A, 0x12AF", cpu.A, 0xF971);
    c = code({ cpu.A = 0xF000; cpu.B = 0xF0; }, "mod A, B", cpu.A, 0x0);
    equals(c.cpu().Flag(Flag::Z), true);

    code({ cpu.A = 0x1234; }, "mod A, 0x12", cpu.A, 0x10);
    code({ cpu.A = 0x1234; }, "mod A, 0x2AF", cpu.A, 0x21A);
    code({ cpu.A = 0x123487AB; }, "mod A, 0x12AF", cpu.A, 0x116C);
    code({ cpu.A = 0x42; }, "inc A", cpu.A, 0x43);
    c = code({ cpu.A = 0xFFFFFFFF; }, "inc A", cpu.A, 0x0); // inc A (overflow)
    equals(c.cpu().Flag(Flag::Y), true);
    equals(c.cpu().Flag(Flag::Z), true);

    code({ cpu.A = 0x42; }, "dec A", cpu.A, 0x41);
    c = code({ cpu.A = 0x0; }, "dec A", cpu.A, 0xFFFFFFFF); // dec A (underflow)
    equals(c.cpu().Flag(Flag::Z), false);
}


static void branches()
{
    cout << "# branches\n";

    code({ cpu.setFlag(Flag::Z, true); cpu.A = 0x1000; }, "bz A", cpu.PC, 0x1000);
    code({ cpu.A = 0x1000; }, "bz A", cpu.PC, 0x2); // bz A (false)
    code({ cpu.setFlag(Flag::Z, true); }, "bz 0x1000", cpu.PC, 0x1000);
    code({ cpu.A = 0x1000; }, "bnz A", cpu.PC, 0x1000);
    code({}, "bnz 0x1000", cpu.PC, 0x1000);
    code({ cpu.setFlag(Flag::S, true); cpu.A = 0x1000; }, "bneg A", cpu.PC, 0x1000);
    code({ cpu.A = 0x1000; }, "bneg A", cpu.PC, 0x2); // bneg A (false)
    code({ cpu.setFlag(Flag::S, true); }, "bneg 0x1000", cpu.PC, 0x1000);
    code({ cpu.A = 0x1000; }, "bpos A", cpu.PC, 0x1000);
    code({}, "bpos 0x1000", cpu.PC, 0x1000);
    code({}, "jmp 0x12345678", cpu.PC, 0x12345678);
}


static void stack()
{
    cout << "# stack\n";

    LuisaVM comp;
    CPU& cpu = comp.cpu();

    cpu.SP = 0xFFF;
    cpu.A = 0xABCDEF12;

    auto compile = [&comp](uint32_t pos, string const& code) {
        vector<uint8_t> data; // TODO = Assembler().AssembleString("test", "section .text\n" + code);
        for(size_t i=0; i<data.size(); ++i) {
            comp.Set(pos+i, data[i]);
        }
    };
    compile(0x0, "pushb A");
    compile(0x2, "pushb 0x12");
    compile(0x4, "pushw A");
    compile(0x6, "pushd A");
    compile(0x8, "popd B");
    compile(0xA, "popw B");
    compile(0xC, "popb B");
    compile(0xE, "popx 1");

    comp.Step();
    equals(comp.Get(0xFFF), 0x12, "pushb A");
    equals(cpu.PC, 0x2, "PC");
    equals(cpu.SP, 0xFFE, "SP");

    comp.Step();
    equals(comp.Get(0xFFE), 0x12, "pushb 0x12");
    equals(cpu.SP, 0xFFD, "SP");

    comp.Step();
    equals(comp.Get16(0xFFC), 0xEF12);
    equals(comp.Get(0xFFD), 0xEF);
    equals(comp.Get(0xFFC), 0x12);
    equals(cpu.SP, 0xFFB, "SP");

    comp.Step();
    equals(comp.Get32(0xFF8), 0xABCDEF12);
    equals(cpu.SP, 0xFF7, "SP");

    comp.Step();
    equals(cpu.B, 0xABCDEF12, "popd B");

    comp.Step();
    equals(cpu.B, 0xEF12, "popw B");

    comp.Step();
    equals(cpu.B, 0x12, "popb B");

    comp.Step();
    equals(cpu.SP, 0xFFF, "popx 1");
}

static void stack_allreg()
{
    cout << "# stack (all registers)\n";

    LuisaVM comp;
    CPU& cpu = comp.cpu();

    cpu.SP = 0xFFF;
    cpu.A = 0xA1B2C3E4;
    cpu.B = 0xFFFFFFFF;
    
    comp.Set(0, Assembler().AssembleString("test", "section .text\npush.a")[0]);
    comp.Set(1, Assembler().AssembleString("test", "section .text\npop.a")[0]);

    comp.Step();
    equals(cpu.SP, 0xFCF);
    equals(comp.Get32(0xFFC), 0xA1B2C3E4, "A is saved");
    equals(comp.Get32(0xFF8), 0xFFFFFFFF, "B is saved");

    cpu.A = cpu.B = 0;
    comp.Step();
    equals(cpu.SP, 0xFFF);
    equals(cpu.A, 0xA1B2C3E4, "A is restored");
    equals(cpu.B, 0xFFFFFFFF, "B is restored");
}


static void others()
{
    code({}, "nop", cpu.PC, 1);
    // TODO - dgb & halt
}


static void cpu_tests()
{
    cout << "#\n";
    cout << "# cpu\n";
    cout << "#\n";

    cpu_santiy();
    mov();
    movb();
    movw();
    movd();
    swap();
    logic();
    integer_math();
    branches();
    stack();
    stack_allreg();
    others();
}

// }}}

}  // namespace luisavm
