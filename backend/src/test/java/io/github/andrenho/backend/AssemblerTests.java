package io.github.andrenho.backend;

import io.github.andrenho.backend.assembler.Assembler;
import io.github.andrenho.backend.assembler.CompilationError;
import io.github.andrenho.backend.assembler.CompiledCode;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

class AssemblerTests {

    private List<Byte> bytes(int... values) {
        ArrayList<Byte> r = new ArrayList<>();
        for (int value : values) {
            if (value < 0 || value > 255)
                throw new IndexOutOfBoundsException();
            r.add((byte) value);
        }
        return r;
    }

    private void testCommand(String cmd, int... values) throws CompilationError {
        CompiledCode cc = Assembler.compile(".SECTION text\n" + cmd);
        assertEquals(cc.getCode(), bytes(values));
    }

    @Test
    void testSimple() throws CompilationError {
        assertEquals(Assembler.compile(
                ".SECTION text  \n" +
                        "   nop     ; this is a comment"),
                new CompiledCode(bytes(0x7b)));
    }

    @Test
    void testValidCommands() throws CompilationError {
        testCommand("pop 2", 0x76, 0x2);
        testCommand("mov A, 0xABCD", 0x03, 0x00, 0xCD, 0xAB);
        testCommand("mov A, B", 0x01, 0x00, 0x01);
        testCommand("movb [A], 0x42", 0x0C, 0x00, 0x42);
        testCommand("movw [0x42], K", 0x25, 0x42, 0x00, 0x00, 0x00, 0x0A);
        testCommand("or C, 0x1234", 0x2F, 0x02, 0x34, 0x12);
        testCommand("not B", 0x41, 0x1);
        testCommand("ret", 0x74);
        testCommand("push.a", 0x7E);
        testCommand("movd B, [0xf0016018]", 0x0A, 0x01, 0x18, 0x60, 0x01, 0xf0);
        testCommand("halt", 0x88);
        testCommand("dbg", 0x89);
        testCommand("swap A, B", 0x8A, 0x0, 0x1);
        testCommand("cmp C", 0x8B, 0x2);
    }

    @Test
    void testPromotion() throws CompilationError {
        testCommand("movd [A], 0x1", 0x1E, 0x00, 0x01, 0x00, 0x00, 0x00);
        testCommand("bz 0x12", 0x5D, 0x12, 0x00, 0x00, 0x00);
        testCommand("movb A, [0x1000]", 0x06, 0x00, 0x00, 0x10, 0x00, 0x00);
    }

    /*
    @Test
    public void testUseful() throws CompilationError {
        assertEquals(Assembler.compile(
                ".SECTION text   \n" +
                "mov     A, 0x4\n" +
                "movb    A, [B]\n" +
                "movb    A, [0x12345678]\n" +
                "movd    [0xABCDEF01], [0x12345678]\n" +
                "or      K, 0x4212\n" +
                "not     F\n" +
                "bz      0x42\n" +
                "ret"), new CompiledCode(bytes(
                        0x02, 0x00, 0x04,
                        0x05, 0x00, 0x01,
                        0x06, 0x00, 0x78, 0x56, 0x34, 0x12,
                        0x2C, 0x01, 0xEF, 0xCD, 0xAB, 0x78, 0x56, 0x34, 0x12,
                        0x2F, 0x0A, 0x12, 0x42,
                        0x41, 0x05,
                        0x5D, 0x42, 0x00, 0x00, 0x00,
                        0x74)));
    }
    */

    // see https://github.com/andrenho/luisavm_old/blob/gh-pages/tools/assembler
}
