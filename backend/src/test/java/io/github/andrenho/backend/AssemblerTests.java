package io.github.andrenho.backend;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import io.github.andrenho.backend.assembler.Assembler;
import io.github.andrenho.backend.assembler.CompiledCode;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.assertEquals;

public class AssemblerTests {

    @Test
    public void testSimple() {
        assertEquals(Assembler.compile(
               ".SECTION text  \n" +
                "   nop     ; this is a comment"),
                new CompiledCode(
                        new ArrayList<Byte>(Arrays.asList((byte)0x7b))));

    }
}
