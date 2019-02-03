package io.github.andrenho.backend.assembler;

public class Linker {

    public static byte[] link(CompiledCode... codes) {
        if (codes.length != 1)
            throw new RuntimeException("More than one source not yet implemented.");
        byte[] bytes = new byte[codes[0].getCode().size()];
        int i = 0;
        for (Byte b : codes[0].getCode())
            bytes[i++] = b;
        return bytes;
    }

}
