package io.github.andrenho.backend.assembler;

import java.util.ArrayList;
import java.util.List;

enum ParameterType {
    Register, V8, V16, V32, IndirectRegister, IndirectAddress;

    public boolean isImmediate() {
        return this == V8 || this == V16 || this == V32;
    }

    public ParameterType promote() {
        return (this == V8 || this == V16) ? V32 : this;
    }
}

class Parameter {

    @Override
    public String toString() {
        return "Parameter{" +
                "type=" + type +
                ", bytes=" + bytes +
                '}';
    }

    Parameter(String p, CompiledCode cc, int nline) throws CompilationError {
        p = p.toLowerCase();
        try {
            long value;
            // TODO - negative hex?
            if (p.startsWith("0x"))
                value = Long.parseLong(p.substring(2).replace("_", ""), 16);
            else if (p.startsWith("0b"))
                value = Long.parseLong(p.substring(2).replace("_", ""), 2);
            else if (p.startsWith("0"))
                value = Long.parseLong(p.substring(1).replace("_", ""), 8);
            else
                value = Long.parseLong(p.replace("_", ""));
            if (value < 0) {
                type = ParameterType.V32;
                bytes.add((byte) (value & 0xFF));
                bytes.add((byte) ((value >> 8) & 0xFF));
                bytes.add((byte) ((value >> 16) & 0xFF));
                bytes.add((byte) (value >> 24));
            } else if (value <= 0xFF) {
                type = ParameterType.V8;
                bytes.add((byte) value);
            } else if (value <= 0xFFFF) {
                type = ParameterType.V16;
                bytes.add((byte) (value & 0xFF));
                bytes.add((byte) (value >> 8));
            } else if (value <= 0xFFFFFFFFL) {
                type = ParameterType.V32;
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
            } else if (p.length() >= 3 && p.charAt(0) == '[' && p.charAt(p.length() - 1) == ']') {
                Parameter pi = new Parameter(p.substring(1, p.length() - 1), cc, nline);
                if (pi.type == ParameterType.Register) {
                    type = ParameterType.IndirectRegister;
                    bytes.addAll(pi.getBytes());
                } else if (pi.type.isImmediate()) {
                    type = ParameterType.IndirectAddress;
                    bytes.addAll(pi.promoteToV32().getBytes());
                } else {
                    throw new CompilationError("Invalid indirect parameter in line " + nline);
                }
            } else {
                throw new CompilationError("Invalid parameter in line " + nline);
            }
        }
    }

    Parameter promoteToV32() {
        List<Byte> bytes = new ArrayList<>();
        switch (type) {
            case V8:
                bytes.add(this.bytes.get(0));
                bytes.add((byte) 0);
                bytes.add((byte) 0);
                bytes.add((byte) 0);
                return new Parameter(ParameterType.V32, bytes);
            case V16:
                bytes.add(this.bytes.get(0));
                bytes.add(this.bytes.get(1));
                bytes.add((byte) 0);
                bytes.add((byte) 0);
                return new Parameter(ParameterType.V32, bytes);
            case V32:
                return this;
            default:
                throw new RuntimeException("Invalid value.");
        }
    }

    ParameterType getType() {
        return type;
    }

    List<Byte> getBytes() {
        return bytes;
    }

    @SuppressWarnings("SameParameterValue")
    private Parameter(ParameterType type, List<Byte> bytes) {
        this.type = type;
        this.bytes = bytes;
    }

    private ParameterType type;
    private List<Byte> bytes = new ArrayList<>();
}

