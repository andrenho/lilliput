#!/usr/bin/env lua

-- Features:
--   * areas
--   * text compilation
--   * data, bss (db, rb, etc...)
--   * labels and local labels
--   * constants
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

function compile(source)
    return {}   -- TODO
end

-- }}}

-- {{{ TESTS

function test(name, code, expected_binary)
    local binary = compile(code)
    local eq = true
    for i,d in ipairs(binary) do
        if d ~= expected_binary[i] then eq = false; break end
    end
    if not eq or #binary ~= #expected_binary then
        print("[ERR] '"..name.."' failed.")
        io.stdout:write("Expected: { ")
        for _,d in ipairs(expected_binary) do io.stdout:write(string.format("%02X, ", d)) end
        io.stdout:write(" }\nFound:    {")
        for _,d in ipairs(binary) do io.stdout:write(string.format("%02X, ", d)) end
        print("}")
        error('unexpected test result', 2)
    else
        print("[OK] "..name)
    end
end

function run_tests()
    test('test', '', { 0x34, 0x12 })
end

-- }}}

-- {{{ PARSE OPTIONS

function display_help()
    print("las [OPTIONS] [SOURCE_FILE]")
    print("Options:")
    print("  -o FILENAME    binary output filename")
    print("  -t             run tests")
    print("  -h             display this help")
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
            io.stderr:write("Unrecognized switch '"..switch.."'.\n")
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
        local data = compile(source)
        local fout = io.open(outfile, 'wb')
        fout:write(string.char(table.unpack(data)))
        fout:close()
    else
        io.stderr:write("Source file not defined.\n")
        os.exit(false)
    end
end

-- }}}
