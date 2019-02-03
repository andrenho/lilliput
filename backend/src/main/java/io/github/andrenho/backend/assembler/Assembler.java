package io.github.andrenho.backend.assembler;

import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Assembler {

    static private Pattern p_section = Pattern.compile("^\\.SECTION\\s+(\\w+)$"),
                           p_commands = Pattern.compile("^(\\w[\\w\\d]*:)?\\s*(.+)$"),
                           p_command = Pattern.compile("^(\\w[\\w\\d]+)(?:\\s+(.+?)\\s*(?:,\\s*(.+?))?)?$");


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

            // remove comments
            line = removeComments(line);
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

    private static String removeComments(String line) {
        Character quote = null;
        for (int i = 0; i < line.length(); ++i) {
            if (quote != null) {
                if (line.charAt(i) == quote)
                    quote = null;
            } else {
                // TODO: skip '\'
                char c = line.charAt(i);
                if (c == '"' || c == '\'')
                    quote = c;
                else if (c == ';')
                    return line.substring(0, i-1);
            }
        }
        return line;
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

        /*
        System.out.println(command);
        for (int i=0; i<=m_cmd.groupCount(); ++i) {
            System.out.println(i + ": " + m_cmd.group(i));
        }
        */

        String cmd = m_cmd.group(1).toLowerCase();
        Parameter op1 = null, op2 = null;
        if (m_cmd.group(2) != null)
            op1 = new Parameter(m_cmd.group(2), cc, nline);
        if (m_cmd.group(3) != null)
            op2 = new Parameter(m_cmd.group(3), cc, nline);

        int before = cc.getCode().size();
        cc.getCode().addAll(Commands.find(cmd, op1, op2, nline));
        return npos + cc.getCode().size() - before;
    }
}
