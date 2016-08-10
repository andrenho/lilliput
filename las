#!/usr/bin/env lua

-- Features:
--   * areas
--   * text compilation
--   * data, bss (db, rb, etc...)
--   * labels and local labels
--   * constants
--   * comments
--   * include files

-- {{{ PREPARATION/DEBUGGING

-- {{{ strict 

local mt = getmetatable(_G)
if mt == nil then
  mt = {}
  setmetatable(_G, mt)
end

__STRICT = true
mt.__declared = {}

mt.__newindex = function (t, n, v)
  if __STRICT and not mt.__declared[n] then
   local w = debug.getinfo(2, "S").what
   if w ~= "main" and w ~= "C" then
     error("assign to undeclared variable '"..n.."'", 2)
   end
   mt.__declared[n] = true
  end
  rawset(t, n, v)
end
  
mt.__index = function (t, n)
  if not mt.__declared[n] and debug.getinfo(2, "S").what ~= "C" then
   error("variable '"..n.."' is not declared", 2)
  end
  return rawget(t, n)
end

function global(...)
   for _, v in ipairs{...} do mt.__declared[v] = true end
end

-- }}}

-- {{{ inspection
function inspect(data)
   if type(data) == 'string' then
      return '"'..data..'"'
   elseif type(data) == 'table' then
      local s = {}
      for i,v in ipairs(data) do s[#s+1] = inspect(v) end
      for k,v in pairs(data) do
         if type(k) ~= 'number' then
            s[#s+1] = '['..inspect(k)..'] = '..inspect(v)
         end
      end
      return '{ '..table.concat(s, ', ')..' }'
   else
      return tostring(data)
   end
end

function p(data)
   print(inspect(data))
end

-- }}}

-- {{{ helper functions
-- }}}

-- }}}

-- {{{ ASSEMBLER

local opcodes = {  --{{{ ...  }
    -- movement
    [0x01] = { instruction = 'mov', parameters = { 'reg', 'reg' } },
    [0x02] = { instruction = 'mov', parameters = { 'reg', 'v8' } },
    [0x03] = { instruction = 'mov', parameters = { 'reg', 'v16' } },
    [0x04] = { instruction = 'mov', parameters = { 'reg', 'v32' } },

    [0x05] = { instruction = 'movb', parameters = { 'reg', 'regind' } },
    [0x06] = { instruction = 'movb', parameters = { 'reg', 'indv32' } },
    [0x07] = { instruction = 'movb', parameters = { 'regind', 'reg' } },
    [0x08] = { instruction = 'movb', parameters = { 'regind', 'v8' } },
    [0x09] = { instruction = 'movb', parameters = { 'regind', 'regind' } },
    [0x0A] = { instruction = 'movb', parameters = { 'regind', 'indv32' } },
    [0x0B] = { instruction = 'movb', parameters = { 'indv32', 'reg' } },
    [0x0C] = { instruction = 'movb', parameters = { 'indv32', 'v8' } },
    [0x0D] = { instruction = 'movb', parameters = { 'indv32', 'regind' } },
    [0x0E] = { instruction = 'movb', parameters = { 'indv32', 'indv32' } },

    [0x0F] = { instruction = 'movw', parameters = { 'reg', 'regind' } },
    [0x10] = { instruction = 'movw', parameters = { 'reg', 'indv32' } },
    [0x11] = { instruction = 'movw', parameters = { 'regind', 'reg' } },
    [0x12] = { instruction = 'movw', parameters = { 'regind', 'v16' } },
    [0x13] = { instruction = 'movw', parameters = { 'regind', 'regind' } },
    [0x14] = { instruction = 'movw', parameters = { 'regind', 'indv32' } },
    [0x15] = { instruction = 'movw', parameters = { 'indv32', 'reg' } },
    [0x16] = { instruction = 'movw', parameters = { 'indv32', 'v16' } },
    [0x17] = { instruction = 'movw', parameters = { 'indv32', 'regind' } },
    [0x18] = { instruction = 'movw', parameters = { 'indv32', 'indv32' } },

    [0x19] = { instruction = 'movd', parameters = { 'reg', 'regind' } },
    [0x1A] = { instruction = 'movd', parameters = { 'reg', 'indv32' } },
    [0x1B] = { instruction = 'movd', parameters = { 'regind', 'reg' } },
    [0x1C] = { instruction = 'movd', parameters = { 'regind', 'v32' } },
    [0x1D] = { instruction = 'movd', parameters = { 'regind', 'regind' } },
    [0x1E] = { instruction = 'movd', parameters = { 'regind', 'indv32' } },
    [0x1F] = { instruction = 'movd', parameters = { 'indv32', 'reg' } },
    [0x20] = { instruction = 'movd', parameters = { 'indv32', 'v32' } },
    [0x21] = { instruction = 'movd', parameters = { 'indv32', 'regind' } },
    [0x22] = { instruction = 'movd', parameters = { 'indv32', 'indv32' } },

    [0x23] = { instruction = 'swap', parameters = { 'reg', 'reg' } },

    -- logic
    [0x24] = { instruction = 'or', parameters = { 'reg', 'reg' } },
    [0x25] = { instruction = 'or', parameters = { 'reg', 'v8' } },
    [0x26] = { instruction = 'or', parameters = { 'reg', 'v16' } },
    [0x27] = { instruction = 'or', parameters = { 'reg', 'v32' } },
    [0x28] = { instruction = 'xor', parameters = { 'reg', 'reg' } },
    [0x29] = { instruction = 'xor', parameters = { 'reg', 'v8' } },
    [0x2A] = { instruction = 'xor', parameters = { 'reg', 'v16' } },
    [0x2B] = { instruction = 'xor', parameters = { 'reg', 'v32' } },
    [0x2C] = { instruction = 'and', parameters = { 'reg', 'reg' } },
    [0x2D] = { instruction = 'and', parameters = { 'reg', 'v8' } },
    [0x2E] = { instruction = 'and', parameters = { 'reg', 'v16' } },
    [0x2F] = { instruction = 'and', parameters = { 'reg', 'v32' } },
    [0x30] = { instruction = 'shl', parameters = { 'reg', 'reg' } },
    [0x31] = { instruction = 'shl', parameters = { 'reg', 'v8' } },
    [0x32] = { instruction = 'shr', parameters = { 'reg', 'reg' } },
    [0x33] = { instruction = 'shr', parameters = { 'reg', 'v8' } },
    [0x34] = { instruction = 'not', parameters = { 'reg', } },

    -- arithmetic
    [0x35] = { instruction = 'add', parameters = { 'reg', 'reg' } },
    [0x36] = { instruction = 'add', parameters = { 'reg', 'v8' } },
    [0x37] = { instruction = 'add', parameters = { 'reg', 'v16' } },
    [0x38] = { instruction = 'add', parameters = { 'reg', 'v32' } },
    [0x39] = { instruction = 'sub', parameters = { 'reg', 'reg' } },
    [0x3A] = { instruction = 'sub', parameters = { 'reg', 'v8' } },
    [0x3B] = { instruction = 'sub', parameters = { 'reg', 'v16' } },
    [0x3C] = { instruction = 'sub', parameters = { 'reg', 'v32' } },
    [0x3D] = { instruction = 'cmp', parameters = { 'reg', 'reg' } },
    [0x3E] = { instruction = 'cmp', parameters = { 'reg', 'v8' } },
    [0x3F] = { instruction = 'cmp', parameters = { 'reg', 'v16' } },
    [0x40] = { instruction = 'cmp', parameters = { 'reg', 'v32' } },
    [0x41] = { instruction = 'cmp', parameters = { 'reg', } },
    [0x42] = { instruction = 'mul', parameters = { 'reg', 'reg' } },
    [0x43] = { instruction = 'mul', parameters = { 'reg', 'v8' } },
    [0x44] = { instruction = 'mul', parameters = { 'reg', 'v16' } },
    [0x45] = { instruction = 'mul', parameters = { 'reg', 'v32' } },
    [0x46] = { instruction = 'idiv', parameters = { 'reg', 'reg' } },
    [0x47] = { instruction = 'idiv', parameters = { 'reg', 'v8' } },
    [0x48] = { instruction = 'idiv', parameters = { 'reg', 'v16' } },
    [0x49] = { instruction = 'idiv', parameters = { 'reg', 'v32' } },
    [0x4A] = { instruction = 'mod', parameters = { 'reg', 'reg' } },
    [0x4B] = { instruction = 'mod', parameters = { 'reg', 'v8' } },
    [0x4C] = { instruction = 'mod', parameters = { 'reg', 'v16' } },
    [0x4D] = { instruction = 'mod', parameters = { 'reg', 'v32' } },
    [0x4E] = { instruction = 'inc', parameters = { 'reg', } },
    [0x4F] = { instruction = 'dec', parameters = { 'reg', } },

    -- jumps
    [0x50] = { instruction = 'bz', parameters = { 'reg', } },
    [0x51] = { instruction = 'bz', parameters = { 'v32', } },
    [0x52] = { instruction = 'bnz', parameters = { 'reg', } },
    [0x53] = { instruction = 'bnz', parameters = { 'v32', } },
    [0x54] = { instruction = 'bneg', parameters = { 'reg', } },
    [0x55] = { instruction = 'bneg', parameters = { 'v32', } },
    [0x56] = { instruction = 'bpos', parameters = { 'reg', } },
    [0x57] = { instruction = 'bpos', parameters = { 'v32', } },
    [0x58] = { instruction = 'bgt', parameters = { 'reg', } },
    [0x59] = { instruction = 'bgt', parameters = { 'v32', } },
    [0x5A] = { instruction = 'bgte', parameters = { 'reg', } },
    [0x5B] = { instruction = 'bgte', parameters = { 'v32', } },
    [0x5C] = { instruction = 'blt', parameters = { 'reg', } },
    [0x5D] = { instruction = 'blt', parameters = { 'v32', } },
    [0x5E] = { instruction = 'blte', parameters = { 'reg', } },
    [0x5F] = { instruction = 'blte', parameters = { 'v32', } },
    [0x60] = { instruction = 'bv', parameters = { 'reg', } },
    [0x61] = { instruction = 'bv', parameters = { 'v32', } },
    [0x62] = { instruction = 'bnv', parameters = { 'reg', } },
    [0x63] = { instruction = 'bnv', parameters = { 'v32', } },

    [0x64] = { instruction = 'jmp', parameters = { 'reg', } },
    [0x65] = { instruction = 'jmp', parameters = { 'v32', } },
    [0x66] = { instruction = 'jsr', parameters = { 'reg', } },
    [0x67] = { instruction = 'jsr', parameters = { 'v32', } },
    [0x68] = { instruction = 'ret', parameters = {} },
    [0x69] = { instruction = 'iret', parameters = {} },

    -- stack
    [0x6A] = { instruction = 'pushb', parameters = { 'reg', } },
    [0x6B] = { instruction = 'pushb', parameters = { 'v8', } },
    [0x6C] = { instruction = 'pushw', parameters = { 'reg', } },
    [0x6D] = { instruction = 'pushw', parameters = { 'v16', } },
    [0x6E] = { instruction = 'pushd', parameters = { 'reg', } },
    [0x6F] = { instruction = 'pushd', parameters = { 'v32', } },
    [0x70] = { instruction = 'push.a', parameters = {} },
    [0x71] = { instruction = 'popb', parameters = { 'reg', } },
    [0x72] = { instruction = 'popw', parameters = { 'reg', } },
    [0x73] = { instruction = 'popd', parameters = { 'reg', } },
    [0x74] = { instruction = 'pop.a', parameters = {} },
    [0x75] = { instruction = 'popx', parameters = { 'reg', } },
    [0x76] = { instruction = 'popx', parameters = { 'v8', } },
    [0x77] = { instruction = 'popx', parameters = { 'v16', } },

    -- other
    [0x78] = { instruction = 'nop', parameters = {} },
    [0x79] = { instruction = 'halt', parameters = {} },
    [0x7A] = { instruction = 'dbg', parameters = {} },
}  --}}}

local registers = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, i=8, j=9, k=10, l=11, fp=12, sp=13, pc=14, fl=15 }

function assembler_error(assembler, msg)  --{{{
   error('** '..msg..' in '..assembler.current_file..':'..assembler.nline..' -> '..assembler.current_line)
end  --}}}

function convert_value(par)  --{{{
   if par:match('^%d+$') then
      return tonumber(par, 10)
   elseif par:match('^0[xX]%x+$') then
      return tonumber(par:sub(3, -1), 16)
   elseif par:match('^0b[01]+$') then
      return tonumber(par:sub(3, -1), 2)
   else
      return nil
   end
end  --}}}

function preproc(assembler, line)  --{{{
   local cmd, def, val = line:gmatch('([^%s]+)%s+([^%s]+)%s+([^%s]+)')()
   if cmd ~= '%define' then
      assembler_error('Unknown directive')
   end
   assembler.constants[def] = val
end  --}}}

function add_label(assembler, lbl) --{{{
   -- TODO - local, global labels
   local pos
   if assembler.section == '.text' then
      pos = #assembler.text
   elseif assembler.section == '.data' then
      pos = #assembler.data
   elseif assembler.section == '.bss' then
      pos = assembler.bss_sz
   else
      assembler_error(assembler, 'Invalid section')
   end
   assembler.labels[lbl] = { section=assembler.section, pos=pos }
end --}}}

function replace_constants(assembler, line)  --{{{
   for k,v in pairs(assembler.constants) do
      line = line:gsub(k, v, 1, -1)
   end
   return line
end  --}}}

function add_data(assembler, sz, data)  --{{{
   for _,t in ipairs(data) do
      local value = convert_value(t)
      if value then
         if sz == 'b' then
            if value > 0xFF then assembler_error('Value too large') end
            table.insert(assembler.data, value)
         elseif sz == 'w' then
            if value > 0xFFFF then assembler_error('Value too large') end
            table.insert(assembler.data, value & 0xFF)
            table.insert(assembler.data, value >> 8)
         elseif sz == 'd' then
            if value > 0xFFFFFFFF then assembler_error('Value too large') end
            table.insert(assembler.data, value & 0xFF)
            table.insert(assembler.data, (value >> 8) & 0xFF)
            table.insert(assembler.data, (value >> 16) & 0xFF)
            table.insert(assembler.data, (value >> 24) & 0xFF)
         else
            assert(false, "we shouldn't have gotten here")
         end
      else
         assembler_error('Invalid data')
      end
   end
end  --}}}

function add_bss(assembler, sz, data)  --{{{
   local n = convert_value(data)
   if not n then
      assembler_error('Invalid BSS size')
   else
      if sz == 'w' then n = n*2 elseif sz == 'd' then n = n*4 end
      assembler.bss_sz = assembler.bss_sz + n
   end
end  --}}}

function add_ascii(assembler, data, zero)  --{{{
   for _,c in ipairs(table.pack(data:byte(1,-1))) do
      table.insert(assembler.data, c)
   end
   if zero then
      table.insert(assembler.data, 0)
   end
end  --}}}

function add_instruction(assembler, inst, pars)  --{{{
   -- find parameters types and values
   local ptype, pvalue, plabel = {}, {}, {}
   for _,par in ipairs(pars) do
      local pt, value, label
      if registers[par:lower()] then
         pt = 'reg'
         value = registers[par:lower()]
      elseif registers[(par:match('^%[(%a%a?)%]$') or ''):lower()] then
         pt = 'regind'
         value = registers[(par:match('^%[(%a%a?)%]$') or ''):lower()]
      elseif par:match('^%d+$') or par:match('^0[xX]%x+$') or par:match('^0b[01]+$') then
         value = assert(convert_value(par))
         if value <= 0xFF then
            pt = 'v8'
         elseif value <= 0xFFFF then
            pt = 'v16'
         elseif value <= 0xFFFFFFFF then
            pt = 'v32'
         else
            assembler_error(assembler, 'Overflow')
            os.exit(false)
         end
      elseif par:match('^%[%d+%]$') or par:match('^%[0[xX]%x+%]$') or par:match('^%[0b[01]+%]$') then
         pt = 'indv32'
         value = assert(convert_value(par:gsub('[%[%]]', '')))
      elseif par:match('%[%w[%a_]*%]') then
         label = par:match('^%[%w[%a_]*%]$')
         value = 0
         pt = 'indv32'
         label = true
      elseif par:match('%w[%a_]*') then
         label = par
         value = 0
         pt = 'v32'
      else 
         assembler_error(assembler, 'Invalid parameter')
      end
      ptype[#ptype+1] = pt
      pvalue[#pvalue+1] = value
      plabel[#plabel+1] = label
   end

   -- find instruction
   for k,v in pairs(opcodes) do
      if inst == v.instruction and ptype[1] == v.parameters[1] and ptype[2] == v.parameters[2] then
         table.insert(assembler.text, k)
         goto found
      end
   end
   assembler_error(assembler, 'Instruction not found')

::found::

   -- add parameters
   if ptype[1]:sub(1,3) == 'reg' and ptype[2]:sub(1,3) == 'reg' then
      table.insert(assembler.text, (pvalue[1] << 4) | pvalue[2])
   else
      for i=1,2 do
         if plabel[i] then
            table.insert(assembler.labels_ref, { label=plabel[i], pos=#assembler.text })
         end
         if ptype[i] then
            if ptype[i]:sub(1,3) == 'reg' or ptype[i] == 'v8' then
               table.insert(assembler.text, pvalue[i])
            elseif ptype[i] == 'v16' then
               table.insert(assembler.text, pvalue[i] & 0xFF)
               table.insert(assembler.text, pvalue[i] >> 8)
            elseif ptype[i] == 'v32' or ptype == 'indv32' then
               table.insert(assembler.text, pvalue[i] & 0xFF)
               table.insert(assembler.text, (pvalue[i] >> 8) & 0xFF)
               table.insert(assembler.text, (pvalue[i] >> 16) & 0xFF)
               table.insert(assembler.text, (pvalue[i] >> 24) & 0xFF)
            else
               error()
            end
         end
      end
   end

end --}}}

function replace_labels(assembler) --{{{
   for _,v in ipairs(assembler.labels_ref) do
      local label = assembler.labels[v.label]
      if label then
         local addr = label.pos
         if label.section == '.data' then
            addr = addr + #assembler.text
         elseif label.section == '.bss' then
            addr = addr + #assembler.text + #assembler.data
         end
         assembler.text[v.pos+1] = addr & 0xFF
         assembler.text[v.pos+2] = (addr >> 8) & 0xFF
         assembler.text[v.pos+3] = (addr >> 16) & 0xFF
         assembler.text[v.pos+4] = (addr >> 24) & 0xFF
      else
         assembler_error("Label "..v.label.." not found")  -- TODO
      end
   end
end  --}}}

function compile(source, filename)  --{{{
   local assembler = {
      text = {},
      data = {},
      bss_sz = 0,
      nline = 1,
      current_line = '',
      current_file = filename or 'stdin',
      section = '',
      constants = {},
      labels = {},
      labels_ref = {},
   }

   local nline = 1
   for line in source:gmatch("([^\n]+)\n?") do  -- split lines
      assembler.current_line = line
      assembler.nline = nline
      -- remove comments
      local newline = line:match('^(.*);.*$')
      if newline then line = newline end

      -- is it a preprocessor directive?
      if line:sub(1,1) == '%' then
         preproc(assembler, line)
         goto nxt
      end
      do
         -- define section
         local section = line:match('section (%.%w+)')
         if section then
            if section == '.text' or section == '.data' or section == '.bss' then
               assembler.section = section
               goto nxt
            else
               assembler_error(assembler, 'Invalid section')
            end
         end
         -- extract label
         local start, finish, match = line:find("^([%.@]?%a%w*):")
         if start then
            add_label(assembler, match)
            line = line:sub(start)
         end
         -- replace directives
         line = replace_constants(assembler, line)
         -- data
         local sz, datax = line:gmatch('%.d([bwd])%s+(.+)')()
         if sz then
            if assembler.section ~= '.data' then assembly_error('Unexpected token') end
            local data = {}
            for d in datax:gmatch('(%w+),?%s*') do data[#data+1] = d end
            add_data(assembler, sz, data)
            goto nxt
         end
         -- bss
         local sz, data = line:gmatch('%.res([bwd])%s+([[%dxXbB]+)')()
         if sz then
            if assembler.section ~= '.bss' then assembly_error('Unexpected token') end
            add_bss(assembler, sz, data)
            goto nxt
         end
         -- ascii
         local zero, data = line:gmatch('%.ascii(z?)%s+"(.+)"')()
         if zero then
            if assembler.section ~= '.data' then assembly_error('Unexpected token') end
            add_ascii(assembler, data, zero == 'z')
            goto nxt
         end
         -- instruction
         local inst, pars = line:gmatch('([%w%.]+)%s+(.+)')()
         if inst then
            if assembler.section ~= '.text' then assembly_error('Unexpected token') end
            local par = {}
            for p in pars:gmatch('([%w%[%]%.@]+),?%s*') do par[#par+1] = p end
            add_instruction(assembler, inst, par)
            goto nxt
         end
      end

      -- not found, bail out
      assembler_error(assembler, "Syntax error: could not parse line")

::nxt::
      nline = nline+1
   end

   -- replace labels
   replace_labels(assembler)

   -- create binary
   local binary = {}
   for _,t in ipairs({ 'text', 'data' }) do
      for _,data in ipairs(assembler[t]) do 
         assert(data <= 0xFF)
         binary[#binary+1] = data 
      end
   end
   return binary
end -- }}}

-- }}}

-- {{{ TESTS

function test(name, code, expected_binary)
   local binary = compile(code, name)
   local eq = true
   for i,d in ipairs(binary) do
      if d ~= expected_binary[i] then eq = false; break end
   end
   if not eq or #binary ~= #expected_binary then
      print("[ERR] '"..name.."' failed.")
      io.stdout:write("Expected: { ")
      for _,d in ipairs(expected_binary) do io.stdout:write(string.format("%02X, ", d)) end
      io.stdout:write(" }\nFound:   {")
      for _,d in ipairs(binary) do io.stdout:write(string.format("%02X, ", d)) end
      print("}")
      error('unexpected test result', 2)
   else
      print("[OK] "..name)
   end
end

function run_tests()
   ----------------------------
   test('basic command', [[
section .text
mov D, 0x64   ; comment]], { 0x2, 0x3, 0x64 })

   ----------------------------
   test('two commands', [[
section .text
mov F, 0x1234

and F, 0xAB]], { 0x3, 0x5, 0x34, 0x12, 0x2D, 0x5, 0xAB })

   ----------------------------
   test('two registers', [[
section .text
mov B, C]], { 0x1, 0x12 })

   ----------------------------
   test('data', [[
section .text
mov A, B
section .data
.db 0x12, 0x34]], { 0x1, 0x1, 0x12, 0x34 })

   ----------------------------
   test('data + string', [[
section .text
mov A, B
section .data
.dw 0x1234
.ascii "abc"]], { 0x1, 0x1, 0x34, 0x12, 0x61, 0x62, 0x63 })

   ----------------------------
   test('data + string (inverted order)', [[
section .data
.dw 0x1234
section .text
mov A, B
section .data
.asciiz "abc"]], { 0x1, 0x1, 0x34, 0x12, 0x61, 0x62, 0x63, 0x0 })

   ----------------------------
   test('res', [[
section .text
mov A, B
section .bss
.resb 8]], { 0x1, 0x1 })

   ----------------------------
   test('constants', [[
%define TEST 0x1234
section .text
mov A, TEST]], { 0x3, 0x0, 0x34, 0x12 })

   ----------------------------
   test('label', [[
section .text
jmp test
test: mov A, B]], { 0x65, 0x5, 0x0, 0x0, 0x0, 0x1, 0x1 })

   -- TODO - test local label
   -- TODO - label on data
   -- TODO - label on bss
   -- TODO - import files
   -- TODO - test global label
end

-- }}}

-- {{{ PARSE OPTIONS

function display_help()
   print("las [OPTIONS] [SOURCE_FILE]")
   print("Options:")
   print("  -o FILENAME   binary output filename")
   print("  -t          run tests")
   print("  -h          display this help")
   os.exit(true)
end


function parse_commandline()
   local opt = {
      run_tests = false,
      output_file = nil,
      source_file = nil,
   }

   local switch
   while true do
      switch = table.remove(arg, 1)
      if switch == nil then
         break
      elseif switch == '-t' then
         opt.run_tests = true
      elseif switch == '-o' then
         opt.output_file = assert(table.remove(arg, 1), 'output filename needed')
      elseif switch == '-h' then
         display_help()
      elseif switch:sub(1, 1) ~= '-' and #arg == 0 then
         opt.source_file = switch
      else
         io.stderr:write("Unrecognized switch '"..switch.."'\n")
         os.exit(false)
      end
   end

   return opt
end

-- }}}

-- {{{ main

local opt = parse_commandline()
if opt.run_tests then
   run_tests()
else
   if opt.source_file then
      local outfile = opt.output_file or opt.source_file:match("(.+)%..*$")..'.bin'
      local fin = io.open(opt.source_file, 'r')
      local source = fin:read('*all')
      fin:close()
      local data = compile(source, opt.source_file)
      local fout = io.open(outfile, 'wb')
      fout:write(string.char(table.unpack(data)))
      fout:close()
   else
      io.stderr:write("Source file not defined.\n")
      os.exit(false)
   end
end

-- }}}

-- vim: st=3:sts=3:sw=3
