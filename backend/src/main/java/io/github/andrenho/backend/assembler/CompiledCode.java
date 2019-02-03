package io.github.andrenho.backend.assembler;

import java.util.*;

public class CompiledCode {
    CompiledCode() {
    }

    public CompiledCode(List<Byte> code) {
        this.code = code;
    }

    public List<Byte> getCode() {
        return code;
    }

    Map<String, Long> getSymbols() {
        return symbols;
    }

    private Map<String, Long> getUnresolved_symbols() {
        return unresolved_symbols;
    }

    @Override
    public String toString() {
        return "CompiledCode{" +
                "code=" + code +
                ", symbols=" + symbols +
                ", unresolved_symbols=" + unresolved_symbols +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        CompiledCode that = (CompiledCode) o;
        return Objects.equals(getCode(), that.getCode()) &&
                Objects.equals(getSymbols(), that.getSymbols()) &&
                Objects.equals(getUnresolved_symbols(), that.getUnresolved_symbols());
    }

    @Override
    public int hashCode() {
        return Objects.hash(getCode(), getSymbols(), getUnresolved_symbols());
    }

    private List<Byte>        code = new ArrayList<>();
    private Map<String, Long> symbols = new HashMap<>();
    private Map<String, Long> unresolved_symbols = new HashMap<>();

}
