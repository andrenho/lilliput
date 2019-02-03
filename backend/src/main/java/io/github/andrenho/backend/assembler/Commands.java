package io.github.andrenho.backend.assembler;

import java.util.*;

class Command {
    public Command(String name) {
        this.name = name;
    }

    public Command(String name, ParameterType par1) {
        this.name = name;
        this.par1 = par1;
    }

    public Command(String name, ParameterType par1, ParameterType par2) {
        this.name = name;
        this.par1 = par1;
        this.par2 = par2;
    }

    String        name;
    ParameterType par1;
    ParameterType par2;

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Command command = (Command) o;
        return name.equals(command.name) &&
                par1 == command.par1 &&
                par2 == command.par2;
    }

    @Override
    public int hashCode() {
        return Objects.hash(name, par1, par2);
    }
}

public class Commands {
    private static Map<Command, Byte> commands = new HashMap<>();

    static {
        // movement
        commands.put(new Command("mov", ParameterType.Register, ParameterType.Register), (byte) 0x01);
        commands.put(new Command("mov", ParameterType.Register, ParameterType.V8), (byte) 0x02);
        commands.put(new Command("mov", ParameterType.Register, ParameterType.V16), (byte) 0x03);
        commands.put(new Command("mov", ParameterType.Register, ParameterType.V32), (byte) 0x04);
        commands.put(new Command("movb", ParameterType.Register, ParameterType.IndirectRegister), (byte) 0x05);
        commands.put(new Command("movb", ParameterType.Register, ParameterType.IndirectAddress), (byte) 0x06);
        commands.put(new Command("movw", ParameterType.Register, ParameterType.IndirectRegister), (byte) 0x07);
        commands.put(new Command("movw", ParameterType.Register, ParameterType.IndirectAddress), (byte) 0x08);
        commands.put(new Command("movd", ParameterType.Register, ParameterType.IndirectRegister), (byte) 0x09);
        commands.put(new Command("movd", ParameterType.Register, ParameterType.IndirectAddress), (byte) 0x0A);

        commands.put(new Command("movb", ParameterType.IndirectRegister, ParameterType.Register), (byte) 0x0B);
        commands.put(new Command("movb", ParameterType.IndirectRegister, ParameterType.V8), (byte) 0x0C);
        commands.put(new Command("movb", ParameterType.IndirectRegister, ParameterType.IndirectRegister), (byte) 0x0D);
        commands.put(new Command("movb", ParameterType.IndirectRegister, ParameterType.IndirectAddress), (byte) 0x0E);
        commands.put(new Command("movw", ParameterType.IndirectRegister, ParameterType.Register), (byte) 0x0F);
        commands.put(new Command("movw", ParameterType.IndirectRegister, ParameterType.V16), (byte) 0x1A);
        commands.put(new Command("movw", ParameterType.IndirectRegister, ParameterType.IndirectRegister), (byte) 0x1B);
        commands.put(new Command("movw", ParameterType.IndirectRegister, ParameterType.IndirectAddress), (byte) 0x1C);
        commands.put(new Command("movd", ParameterType.IndirectRegister, ParameterType.Register), (byte) 0x1D);
        commands.put(new Command("movd", ParameterType.IndirectRegister, ParameterType.V32), (byte) 0x1E);
        commands.put(new Command("movd", ParameterType.IndirectRegister, ParameterType.IndirectRegister), (byte) 0x1F);
        commands.put(new Command("movd", ParameterType.IndirectRegister, ParameterType.IndirectAddress), (byte) 0x20);

        commands.put(new Command("movb", ParameterType.IndirectAddress, ParameterType.Register), (byte) 0x21);
        commands.put(new Command("movb", ParameterType.IndirectAddress, ParameterType.V8), (byte) 0x22);
        commands.put(new Command("movb", ParameterType.IndirectAddress, ParameterType.IndirectRegister), (byte) 0x23);
        commands.put(new Command("movb", ParameterType.IndirectAddress, ParameterType.IndirectAddress), (byte) 0x24);
        commands.put(new Command("movw", ParameterType.IndirectAddress, ParameterType.Register), (byte) 0x25);
        commands.put(new Command("movw", ParameterType.IndirectAddress, ParameterType.V16), (byte) 0x26);
        commands.put(new Command("movw", ParameterType.IndirectAddress, ParameterType.IndirectRegister), (byte) 0x27);
        commands.put(new Command("movw", ParameterType.IndirectAddress, ParameterType.IndirectAddress), (byte) 0x28);
        commands.put(new Command("movd", ParameterType.IndirectAddress, ParameterType.Register), (byte) 0x29);
        commands.put(new Command("movd", ParameterType.IndirectAddress, ParameterType.V32), (byte) 0x2A);
        commands.put(new Command("movd", ParameterType.IndirectAddress, ParameterType.IndirectRegister), (byte) 0x2B);
        commands.put(new Command("movd", ParameterType.IndirectAddress, ParameterType.IndirectAddress), (byte) 0x2C);

        commands.put(new Command("swap", ParameterType.Register, ParameterType.Register), (byte) 0x8A);

        // logic
        commands.put(new Command("or", ParameterType.Register, ParameterType.Register), (byte) 0x2D);
        commands.put(new Command("or", ParameterType.Register, ParameterType.V8), (byte) 0x2E);
        commands.put(new Command("or", ParameterType.Register, ParameterType.V16), (byte) 0x2F);
        commands.put(new Command("or", ParameterType.Register, ParameterType.V32), (byte) 0x30);
        commands.put(new Command("xor", ParameterType.Register, ParameterType.Register), (byte) 0x31);
        commands.put(new Command("xor", ParameterType.Register, ParameterType.V8), (byte) 0x32);
        commands.put(new Command("xor", ParameterType.Register, ParameterType.V16), (byte) 0x33);
        commands.put(new Command("xor", ParameterType.Register, ParameterType.V32), (byte) 0x34);
        commands.put(new Command("and", ParameterType.Register, ParameterType.Register), (byte) 0x35);
        commands.put(new Command("and", ParameterType.Register, ParameterType.V8), (byte) 0x36);
        commands.put(new Command("and", ParameterType.Register, ParameterType.V16), (byte) 0x37);
        commands.put(new Command("and", ParameterType.Register, ParameterType.V32), (byte) 0x38);
        commands.put(new Command("shl", ParameterType.Register, ParameterType.Register), (byte) 0x39);
        commands.put(new Command("shl", ParameterType.Register, ParameterType.V8), (byte) 0x3A);
        commands.put(new Command("shr", ParameterType.Register, ParameterType.Register), (byte) 0x3D);
        commands.put(new Command("shr", ParameterType.Register, ParameterType.V8), (byte) 0x3E);
        commands.put(new Command("not", ParameterType.Register), (byte) 0x41);

        // arithmetic
        commands.put(new Command("add", ParameterType.Register, ParameterType.Register), (byte) 0x42);
        commands.put(new Command("add", ParameterType.Register, ParameterType.V8), (byte) 0x43);
        commands.put(new Command("add", ParameterType.Register, ParameterType.V16), (byte) 0x44);
        commands.put(new Command("add", ParameterType.Register, ParameterType.V32), (byte) 0x45);
        commands.put(new Command("sub", ParameterType.Register, ParameterType.Register), (byte) 0x46);
        commands.put(new Command("sub", ParameterType.Register, ParameterType.V8), (byte) 0x47);
        commands.put(new Command("sub", ParameterType.Register, ParameterType.V16), (byte) 0x48);
        commands.put(new Command("sub", ParameterType.Register, ParameterType.V32), (byte) 0x49);
        commands.put(new Command("cmp", ParameterType.Register, ParameterType.Register), (byte) 0x4A);
        commands.put(new Command("cmp", ParameterType.Register, ParameterType.V8), (byte) 0x4B);
        commands.put(new Command("cmp", ParameterType.Register, ParameterType.V16), (byte) 0x4C);
        commands.put(new Command("cmp", ParameterType.Register, ParameterType.V32), (byte) 0x4D);
        commands.put(new Command("cmp", ParameterType.Register), (byte) 0x8B);
        commands.put(new Command("mul", ParameterType.Register, ParameterType.Register), (byte) 0x4E);
        commands.put(new Command("mul", ParameterType.Register, ParameterType.V8), (byte) 0x4F);
        commands.put(new Command("mul", ParameterType.Register, ParameterType.V16), (byte) 0x50);
        commands.put(new Command("mul", ParameterType.Register, ParameterType.V32), (byte) 0x51);
        commands.put(new Command("idiv", ParameterType.Register, ParameterType.Register), (byte) 0x52);
        commands.put(new Command("idiv", ParameterType.Register, ParameterType.V8), (byte) 0x53);
        commands.put(new Command("idiv", ParameterType.Register, ParameterType.V16), (byte) 0x54);
        commands.put(new Command("idiv", ParameterType.Register, ParameterType.V32), (byte) 0x55);
        commands.put(new Command("mod", ParameterType.Register, ParameterType.Register), (byte) 0x56);
        commands.put(new Command("mod", ParameterType.Register, ParameterType.V8), (byte) 0x57);
        commands.put(new Command("mod", ParameterType.Register, ParameterType.V16), (byte) 0x58);
        commands.put(new Command("mod", ParameterType.Register, ParameterType.V32), (byte) 0x59);
        commands.put(new Command("inc", ParameterType.Register), (byte) 0x5A);
        commands.put(new Command("dec", ParameterType.Register), (byte) 0x5B);

        // jumps
        commands.put(new Command("bz", ParameterType.Register), (byte) 0x5C);
        commands.put(new Command("bz", ParameterType.V32), (byte) 0x5D);
        commands.put(new Command("beq", ParameterType.Register), (byte) 0x5C);
        commands.put(new Command("beq", ParameterType.V32), (byte) 0x5D);
        commands.put(new Command("bnz", ParameterType.Register), (byte) 0x5E);
        commands.put(new Command("bnz", ParameterType.V32), (byte) 0x5F);
        commands.put(new Command("bneg", ParameterType.Register), (byte) 0x60);
        commands.put(new Command("bneg", ParameterType.V32), (byte) 0x61);
        commands.put(new Command("bpos", ParameterType.Register), (byte) 0x62);
        commands.put(new Command("bpos", ParameterType.V32), (byte) 0x63);
        commands.put(new Command("bgt", ParameterType.Register), (byte) 0x64);
        commands.put(new Command("bgt", ParameterType.V32), (byte) 0x65);
        commands.put(new Command("bgte", ParameterType.Register), (byte) 0x66);
        commands.put(new Command("bgte", ParameterType.V32), (byte) 0x67);
        commands.put(new Command("blt", ParameterType.Register), (byte) 0x68);
        commands.put(new Command("blt", ParameterType.V32), (byte) 0x69);
        commands.put(new Command("blte", ParameterType.Register), (byte) 0x6A);
        commands.put(new Command("blte", ParameterType.V32), (byte) 0x6B);
        commands.put(new Command("bv", ParameterType.Register), (byte) 0x6C);
        commands.put(new Command("bv", ParameterType.V32), (byte) 0x6D);
        commands.put(new Command("bnv", ParameterType.Register), (byte) 0x6E);
        commands.put(new Command("bnv", ParameterType.V32), (byte) 0x6F);
        commands.put(new Command("jmp", ParameterType.Register), (byte) 0x70);
        commands.put(new Command("jmp", ParameterType.V32), (byte) 0x71);
        commands.put(new Command("jsr", ParameterType.Register), (byte) 0x72);
        commands.put(new Command("jsr", ParameterType.V32), (byte) 0x73);
        commands.put(new Command("ret"), (byte) 0x74);
        commands.put(new Command("sys", ParameterType.Register), (byte) 0x75);
        commands.put(new Command("sys", ParameterType.V8), (byte) 0x76);
        commands.put(new Command("iret"), (byte) 0x77);
        commands.put(new Command("sret"), (byte) 0x86);

        // stack
        commands.put(new Command("pushb", ParameterType.Register), (byte) 0x78);
        commands.put(new Command("pushb", ParameterType.V8), (byte) 0x79);
        commands.put(new Command("pushw", ParameterType.Register), (byte) 0x7A);
        commands.put(new Command("pushw", ParameterType.V16), (byte) 0x7B);
        commands.put(new Command("pushd", ParameterType.Register), (byte) 0x7C);
        commands.put(new Command("pushd", ParameterType.V32), (byte) 0x7D);
        commands.put(new Command("push.a"), (byte) 0x7E);
        commands.put(new Command("popb", ParameterType.Register), (byte) 0x7F);
        commands.put(new Command("popw", ParameterType.Register), (byte) 0x80);
        commands.put(new Command("popd", ParameterType.Register), (byte) 0x81);
        commands.put(new Command("pop.a"), (byte) 0x82);
        commands.put(new Command("popx", ParameterType.Register), (byte) 0x83);
        commands.put(new Command("popx", ParameterType.V8), (byte) 0x84);
        commands.put(new Command("popx", ParameterType.V16), (byte) 0x85);

        // other
        commands.put(new Command("nop"), (byte) 0x87);
        commands.put(new Command("halt"), (byte) 0x88);
        commands.put(new Command("dbg"), (byte) 0x89);

        commands.put(new Command("mov", ParameterType.Register, ParameterType.V16), (byte) 0x3);
        commands.put(new Command("pop", ParameterType.Register), (byte) 0x75);
        commands.put(new Command("pop", ParameterType.V8), (byte) 0x76);
        commands.put(new Command("pop", ParameterType.V16), (byte) 0x77);
        commands.put(new Command("nop"), (byte) 0x7B);
    }

    public static List<Byte> find(String command, Parameter p1, Parameter p2, int nline) throws CompilationError {
        List<Byte> r = new ArrayList<>();
        Byte b = commands.get(new Command(command, (p1 != null) ? p1.getType() : null, (p2 != null) ? p2.getType() : null));

        if (b == null) { // not found, try to promote it
            if (p2 != null && p2.getType().isImmediate()) {
                p2 = p2.promoteToV32();
                b = commands.get(new Command(command, p1.getType(), p2.getType()));
            }
            if (b == null && p1 != null && p1.getType().isImmediate()) {
                p1 = p1.promoteToV32();
                b = commands.get(new Command(command, p1.getType().promote()));
            }
        }

        if (b != null) {
            r.add(b);
            if (p1 != null)
                r.addAll(p1.getBytes());
            if (p2 != null)
                r.addAll(p2.getBytes());
        } else {
            throw new CompilationError("Invalid command in line " + nline);
        }
        return r;
    }
}
