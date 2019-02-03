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
        commands.put(new Command("mov", ParameterType.Register, ParameterType.V16), (byte) 0x3);
        commands.put(new Command("pop", ParameterType.Register), (byte) 0x75);
        commands.put(new Command("pop", ParameterType.V8), (byte) 0x76);
        commands.put(new Command("pop", ParameterType.V16), (byte) 0x77);
        commands.put(new Command("nop"), (byte) 0x7B);
    }

    public static List<Byte> find(String command, Parameter p1, Parameter p2, int nline) throws CompilationError {
        List<Byte> r = new ArrayList<>();
        Byte b = commands.get(new Command(command, (p1 != null) ? p1.getType() : null, (p2 != null) ? p2.getType() : null));
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
