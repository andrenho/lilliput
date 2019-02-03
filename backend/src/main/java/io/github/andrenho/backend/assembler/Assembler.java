package io.github.andrenho.backend.assembler;

import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Assembler {

    static String comment = "(\\s*;.*)?";
    static private Pattern p_section = Pattern.compile("\\.SECTION\\s+(\\w+)" + comment),
                           p_commands = Pattern.compile("(\\w[\\w\\d]*:)?\\s*(.+?)[(\\s*;.*)?|$]"),
                           p_command = Pattern.compile("(\\w[\\w\\d]+)(?:\\s+(.+?)\\s*(?:,\\s*(.+?))?)?");


    enum Section { None, Text }

    public static CompiledCode compile(String code) throws CompilationError {

        Section section = Section.None;
        CompiledCode cc = new CompiledCode();

        long npos = 0x0;
        int nline = 1;
        Scanner scan = new Scanner(new StringReader(code));
        while (scan.hasNextLine()) {
            // read line
            String line = scan.nextLine();
            line = line.trim();

            // skip empty lines
            if (line.isEmpty() || line.startsWith(";"))
                continue;

            // match section
            Matcher m_section = p_section.matcher(line);
            if (m_section.find()) {
                switch (m_section.group(1).toLowerCase()) {
                    case "text":
                        section = Section.Text;
                        continue;
                }
            }

            // parse sections
            switch (section) {
                case None:
                    throw new CompilationError("Invalid section in line " + nline);
                case Text:
                    npos = parseTextSection(line, cc, nline, npos);
            }

            ++nline;
        }

        return cc;
    }

    private static long parseTextSection(String line, CompiledCode cc, int nline, long npos) throws CompilationError {
        Matcher m_text = p_commands.matcher(line);
        if (!m_text.find())
            throw new CompilationError("Syntax error in line " + nline);

        String label = m_text.group(1);
        String command = m_text.group(2);

        if (label != null)
            cc.getSymbols().put(label, npos);

        npos = parseCommand(cc, command, nline, npos);

        return npos;
    }

    private static long parseCommand(CompiledCode cc, String command, int nline, long npos) throws CompilationError {
        Matcher m_cmd = p_command.matcher(command);
        if (!m_cmd.find())
            throw new CompilationError("Invalid command in line " + nline);

        String cmd = m_cmd.group(1).toLowerCase();
        Parameter op1 = null, op2 = null;
        if (m_cmd.groupCount() > 1)
            op1 = new Parameter(m_cmd.group(2), cc, nline);
        if (m_cmd.groupCount() > 2)
            op2 = new Parameter(m_cmd.group(3), cc, nline);

        int before = cc.getCode().size();

        switch (cmd) {
            case "nop":
                cc.getCode().add((byte) 0x7B);
                break;
            case "pop":
                if (op2 == null) {
                    if (op1.type == ParameterType.Register)
                        cc.getCode().add((byte) 0x75);
                    else if (op1.type == ParameterType.V8)
                        cc.getCode().add((byte) 0x76);
                    else if (op1.type == ParameterType.V16)
                        cc.getCode().add((byte) 0x77);
                    else
                        throw new CompilationError("Invalid parameter for 'pop' in line " + nline);
                    cc.getCode().addAll(op1.bytes);
                }
            default:
                throw new CompilationError("Invalid command '" + cmd + "' in line " + nline);
        }

        return npos + cc.getCode().size() - before;
    }
}
