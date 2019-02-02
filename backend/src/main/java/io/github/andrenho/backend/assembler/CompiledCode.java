package io.github.andrenho.backend.assembler;

import java.util.*;

public class CompiledCode {
    public CompiledCode() {
    }

    public CompiledCode(List<Byte> code) {
        this.code = code;
    }

    public CompiledCode(List<Byte> code, Map<String, Long> symbols) {
        this.code = code;
        this.symbols = symbols;
    }

    public CompiledCode(List<Byte> code, Map<String, Long> symbols, Map<String, Long> unresolved_symbols) {
        this.code = code;
        this.symbols = symbols;
        this.unresolved_symbols = unresolved_symbols;
    }

    public List<Byte> getCode() {
        return code;
    }

    public void setCode(List<Byte> code) {
        this.code = code;
    }

    public Map<String, Long> getSymbols() {
        return symbols;
    }

    public void setSymbols(Map<String, Long> symbols) {
        this.symbols = symbols;
    }

    public Map<String, Long> getUnresolved_symbols() {
        return unresolved_symbols;
    }

    public void setUnresolved_symbols(Map<String, Long> unresolved_symbols) {
        this.unresolved_symbols = unresolved_symbols;
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
