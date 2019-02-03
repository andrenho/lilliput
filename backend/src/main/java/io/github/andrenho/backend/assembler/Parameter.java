package io.github.andrenho.backend.assembler;

import java.util.ArrayList;
import java.util.List;

enum ParameterType { Register, V8, V16, V32, IndirectRegister, IndirectAddress }

class Parameter {

    public Parameter(String p, CompiledCode cc, int nline) throws CompilationError {
        p = p.toLowerCase();
        try {
            long value;
            if (p.startsWith("0x"))
                value = Long.parseLong(p.substring(2), 16);
            else if (p.startsWith("0b"))
                value = Long.parseLong(p.substring(2), 2);
            else if (p.startsWith("0"))
                value = Long.parseLong(p.substring(1), 8);
            else
                value = Long.parseLong(p);
            if (value <= 0xFF) {
                type = ParameterType.V8;
                bytes.add((byte) value);
            } else if (value <= 0xFFFF) {
                type = ParameterType.V16;
                bytes.add((byte) (value & 0xFF));
                bytes.add((byte) (value >> 8));
            } else if (value <= 0xFFFFFFFF) {
                type = ParameterType.V16;
                bytes.add((byte) (value & 0xFF));
                bytes.add((byte) ((value >> 8) & 0xFF));
                bytes.add((byte) ((value >> 16) & 0xFF));
                bytes.add((byte) (value >> 24));
            } else {
                throw new CompilationError("Number " + p + " too large in line " + nline);
            }
        } catch (NumberFormatException e) {
            if (p.length() == 1 && p.charAt(0) >= 'a' && p.charAt(0) <= 'l') {
                type = ParameterType.Register;
                bytes.add((byte) (p.charAt(0) - 'a'));
            } else {
                throw new Error("Not implemented: " + p);
            }
        }
    }

    public Parameter(ParameterType type, short... bytes) {
        this.type = type;
        for (short b : bytes)
            this.bytes.add((byte) b);
    }

    public ParameterType getType() {
        return type;
    }

    public List<Byte> getBytes() {
        return bytes;
    }

    private ParameterType type;
    private List<Byte> bytes = new ArrayList<>();
}

