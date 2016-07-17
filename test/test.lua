luisavm = require("luisavm")

--luisavm.debug_log(true)

--
-- HELPER FUNCTIONS
--

-- {{{

test_count = 0
err_count = 0

function equals(tested, expect, text)
    test_count = test_count + 1
    if type(tested) == "boolean" then tested = tested and 1 or 0 end
    if type(expect) == "boolean" then expect = expect and 1 or 0 end
    if tested ~= expect then
        print("[\27[31merr\27[0m] " .. text .. " == " .. string.format("0x%X", tested) .. " (expected " .. string.format("0x%X", expect) .. ")")
        err_count = err_count + 1
    else
        print("[\27[32mok\27[0m] " .. text .. " == " .. string.format("0x%X", expect))
    end
end

function computer()
    local computer = luisavm.create_computer(1024 * 1024)
    computer:add_cpu()
    return computer
end

--}}}

-- 
-- COMPILATION
--

local opcodes = {
    -- movement
    [0x01] = { instruction = 'mov', parameters = { 'reg', 'reg' } },
    [0x02] = { instruction = 'mov', parameters = { 'reg', 'v8' } },
    [0x03] = { instruction = 'mov', parameters = { 'reg', 'v16' } },
    [0x04] = { instruction = 'mov', parameters = { 'reg', 'v32' } },
    [0x05] = { instruction = 'movb', parameters = { 'reg', 'regind' } },
    [0x06] = { instruction = 'movb', parameters = { 'reg', 'indv32' } },
    [0x07] = { instruction = 'movw', parameters = { 'reg', 'regind' } },
    [0x08] = { instruction = 'movw', parameters = { 'reg', 'indv32' } },
    [0x09] = { instruction = 'movd', parameters = { 'reg', 'regind' } },
    [0x0A] = { instruction = 'movd', parameters = { 'reg', 'indv32' } },

    [0x0B] = { instruction = 'movb', parameters = { 'regind', 'reg' } },
    [0x0C] = { instruction = 'movb', parameters = { 'regind', 'v8' } },
    [0x0D] = { instruction = 'movb', parameters = { 'regind', 'regind' } },
    [0x0E] = { instruction = 'movb', parameters = { 'regind', 'indv32' } },
    [0x0F] = { instruction = 'movw', parameters = { 'regind', 'reg' } },
    [0x1A] = { instruction = 'movw', parameters = { 'regind', 'v16' } },
    [0x1B] = { instruction = 'movw', parameters = { 'regind', 'regind' } },
    [0x1C] = { instruction = 'movw', parameters = { 'regind', 'indv32' } },
    [0x1D] = { instruction = 'movd', parameters = { 'regind', 'reg' } },
    [0x1E] = { instruction = 'movd', parameters = { 'regind', 'v32' } },
    [0x1F] = { instruction = 'movd', parameters = { 'regind', 'regind' } },
    [0x20] = { instruction = 'movd', parameters = { 'regind', 'indv32' } },

    [0x21] = { instruction = 'movb', parameters = { 'indv32', 'reg' } },
    [0x22] = { instruction = 'movb', parameters = { 'indv32', 'v8' } },
    [0x23] = { instruction = 'movb', parameters = { 'indv32', 'regind' } },
    [0x24] = { instruction = 'movb', parameters = { 'indv32', 'indv32' } },
    [0x25] = { instruction = 'movw', parameters = { 'indv32', 'reg' } },
    [0x26] = { instruction = 'movw', parameters = { 'indv32', 'v16' } },
    [0x27] = { instruction = 'movw', parameters = { 'indv32', 'regind' } },
    [0x28] = { instruction = 'movw', parameters = { 'indv32', 'indv32' } },
    [0x29] = { instruction = 'movd', parameters = { 'indv32', 'reg' } },
    [0x2A] = { instruction = 'movd', parameters = { 'indv32', 'v32' } },
    [0x2B] = { instruction = 'movd', parameters = { 'indv32', 'regind' } },
    [0x2C] = { instruction = 'movd', parameters = { 'indv32', 'indv32' } },

    [0x8A] = { instruction = 'swap', parameters = { 'reg', 'reg' } },

    -- logic
    [0x2D] = { instruction = 'or', parameters = { 'reg', 'reg' } },
    [0x2E] = { instruction = 'or', parameters = { 'reg', 'v8' } },
    [0x2F] = { instruction = 'or', parameters = { 'reg', 'v16' } },
    [0x30] = { instruction = 'or', parameters = { 'reg', 'v32' } },
    [0x31] = { instruction = 'xor', parameters = { 'reg', 'reg' } },
    [0x32] = { instruction = 'xor', parameters = { 'reg', 'v8' } },
    [0x33] = { instruction = 'xor', parameters = { 'reg', 'v16' } },
    [0x34] = { instruction = 'xor', parameters = { 'reg', 'v32' } },
    [0x35] = { instruction = 'and', parameters = { 'reg', 'reg' } },
    [0x36] = { instruction = 'and', parameters = { 'reg', 'v8' } },
    [0x37] = { instruction = 'and', parameters = { 'reg', 'v16' } },
    [0x38] = { instruction = 'and', parameters = { 'reg', 'v32' } },
    [0x39] = { instruction = 'shl', parameters = { 'reg', 'reg' } },
    [0x3A] = { instruction = 'shl', parameters = { 'reg', 'v8' } },
    [0x3D] = { instruction = 'shr', parameters = { 'reg', 'reg' } },
    [0x3E] = { instruction = 'shr', parameters = { 'reg', 'v8' } },
    [0x41] = { instruction = 'not', parameters = { 'reg', } },

    -- arithmetic
    [0x42] = { instruction = 'add', parameters = { 'reg', 'reg' } },
    [0x43] = { instruction = 'add', parameters = { 'reg', 'v8' } },
    [0x44] = { instruction = 'add', parameters = { 'reg', 'v16' } },
    [0x45] = { instruction = 'add', parameters = { 'reg', 'v32' } },
    [0x46] = { instruction = 'sub', parameters = { 'reg', 'reg' } },
    [0x47] = { instruction = 'sub', parameters = { 'reg', 'v8' } },
    [0x48] = { instruction = 'sub', parameters = { 'reg', 'v16' } },
    [0x49] = { instruction = 'sub', parameters = { 'reg', 'v32' } },
    [0x4A] = { instruction = 'cmp', parameters = { 'reg', 'reg' } },
    [0x4B] = { instruction = 'cmp', parameters = { 'reg', 'v8' } },
    [0x4C] = { instruction = 'cmp', parameters = { 'reg', 'v16' } },
    [0x4D] = { instruction = 'cmp', parameters = { 'reg', 'v32' } },
    [0x8B] = { instruction = 'cmp', parameters = { 'reg', } },
    [0x4E] = { instruction = 'mul', parameters = { 'reg', 'reg' } },
    [0x4F] = { instruction = 'mul', parameters = { 'reg', 'v8' } },
    [0x50] = { instruction = 'mul', parameters = { 'reg', 'v16' } },
    [0x51] = { instruction = 'mul', parameters = { 'reg', 'v32' } },
    [0x52] = { instruction = 'idiv', parameters = { 'reg', 'reg' } },
    [0x53] = { instruction = 'idiv', parameters = { 'reg', 'v8' } },
    [0x54] = { instruction = 'idiv', parameters = { 'reg', 'v16' } },
    [0x55] = { instruction = 'idiv', parameters = { 'reg', 'v32' } },
    [0x56] = { instruction = 'mod', parameters = { 'reg', 'reg' } },
    [0x57] = { instruction = 'mod', parameters = { 'reg', 'v8' } },
    [0x58] = { instruction = 'mod', parameters = { 'reg', 'v16' } },
    [0x59] = { instruction = 'mod', parameters = { 'reg', 'v32' } },
    [0x5A] = { instruction = 'inc', parameters = { 'reg', } },
    [0x5B] = { instruction = 'dec', parameters = { 'reg', } },

    -- jumps
    [0x5C] = { instruction = 'bz', parameters = { 'reg', } },
    [0x5D] = { instruction = 'bz', parameters = { 'v32', } },
    [0x5C] = { instruction = 'beq', parameters = { 'reg', } },
    [0x5D] = { instruction = 'beq', parameters = { 'v32', } },
    [0x5E] = { instruction = 'bnz', parameters = { 'reg', } },
    [0x5F] = { instruction = 'bnz', parameters = { 'v32', } },
    [0x60] = { instruction = 'bneg', parameters = { 'reg', } },
    [0x61] = { instruction = 'bneg', parameters = { 'v32', } },
    [0x62] = { instruction = 'bpos', parameters = { 'reg', } },
    [0x63] = { instruction = 'bpos', parameters = { 'v32', } },
    [0x64] = { instruction = 'bgt', parameters = { 'reg', } },
    [0x65] = { instruction = 'bgt', parameters = { 'v32', } },
    [0x66] = { instruction = 'bgte', parameters = { 'reg', } },
    [0x67] = { instruction = 'bgte', parameters = { 'v32', } },
    [0x68] = { instruction = 'blt', parameters = { 'reg', } },
    [0x69] = { instruction = 'blt', parameters = { 'v32', } },
    [0x6A] = { instruction = 'blte', parameters = { 'reg', } },
    [0x6B] = { instruction = 'blte', parameters = { 'v32', } },
    [0x6C] = { instruction = 'bv', parameters = { 'reg', } },
    [0x6D] = { instruction = 'bv', parameters = { 'v32', } },
    [0x6E] = { instruction = 'bnv', parameters = { 'reg', } },
    [0x6F] = { instruction = 'bnv', parameters = { 'v32', } },
    [0x70] = { instruction = 'jmp', parameters = { 'reg', } },
    [0x71] = { instruction = 'jmp', parameters = { 'v32', } },
    [0x72] = { instruction = 'jsr', parameters = { 'reg', } },
    [0x73] = { instruction = 'jsr', parameters = { 'v32', } },
    [0x74] = { instruction = 'ret', parameters = {} },
    [0x75] = { instruction = 'sys', parameters = { 'reg', } },
    [0x76] = { instruction = 'sys', parameters = { 'v8', } },
    [0x77] = { instruction = 'iret', parameters = {} },
    [0x86] = { instruction = 'sret', parameters = {} },

    -- stack
    [0x78] = { instruction = 'pushb', parameters = { 'reg', } },
    [0x79] = { instruction = 'pushb', parameters = { 'v8', } },
    [0x7A] = { instruction = 'pushw', parameters = { 'reg', } },
    [0x7B] = { instruction = 'pushw', parameters = { 'v16', } },
    [0x7C] = { instruction = 'pushd', parameters = { 'reg', } },
    [0x7D] = { instruction = 'pushd', parameters = { 'v32', } },
    [0x7E] = { instruction = 'push.a', parameters = {} },
    [0x7F] = { instruction = 'popb', parameters = { 'reg', } },
    [0x80] = { instruction = 'popw', parameters = { 'reg', } },
    [0x81] = { instruction = 'popd', parameters = { 'reg', } },
    [0x82] = { instruction = 'pop.a', parameters = {} },
    [0x83] = { instruction = 'popx', parameters = { 'reg', } },
    [0x84] = { instruction = 'popx', parameters = { 'v8', } },
    [0x85] = { instruction = 'popx', parameters = { 'v16', } },

    -- other
    [0x87] = { instruction = 'nop', parameters = {} },
    [0x88] = { instruction = 'halt', parameters = {} },
    [0x89] = { instruction = 'dbg', parameters = {} },
}

function compile(comp, code)
    local registers = { A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7, I=8, J=9, K=10, L=11, FP=12, SP=13, PC=14, FL=15 }

    -- parse parameter
    function parse_parameter(par)
        local fst = par:sub(1, 1)
        if fst == '[' then
            if par:sub(2, 2):match('%a') then
                return { type='regind', value={ assert(registers[par]) } }
            else
                local v = math.tointeger(par:sub(2, #par-1))
                return { type='indv32', value={ (v & 0xFF), ((v >> 8) & 0xFF), ((v >> 16) & 0xFF), ((v >> 24) & 0xFF) } }
            end
        elseif fst:match('%a') then
            return { type='reg', value={ assert(registers[par]) } }
        else
            local v = assert(math.tointeger(par:sub(2, #par-1)))
            if v <= 0xFF then
                return { type='v8', value={ v } }
            elseif v <= 0xFFFF then
                return { type='v16', value={ (v & 0xFF), ((v >> 8) & 0xFF) } }
            else
                return { type='v32', value={ (v & 0xFF), ((v >> 8) & 0xFF), ((v >> 16) & 0xFF), ((v >> 24) & 0xFF) } }
            end
        end
        assert(false, "we shouldn't get here")
    end

    -- get instruction and parameters
    local instruction, parameters = code:match("(%w+)%s.*"), {}
    for w in code:gmatch("[%s,]([%w%[%]]+)") do
        parameters[#parameters+1] = parse_parameter(w)
    end

    -- find operator
    for opcode, op in pairs(opcodes) do
        if op.instruction == instruction and #op.parameters == #parameters then
            local eq = true
            for i,p in ipairs(op.parameters) do
                if p ~= parameters[i].type then eq = false end
            end
            if eq then
                comp:set(0, opcode)
                goto found
            end
        end
    end
    assert(false, "opcode from code '" .. code .. "' not found")
::found::
    
    -- add parameters
    if parameters[1].type:sub(1, 3) == 'reg' and parameters[2].type:sub(1, 3) == 'reg' then
        comp:set(1, (parameters[1].value[1] << 4) | parameters[2].value[1])
    else
        local i = 1
        for _,p in ipairs(parameters) do
            for _,v in ipairs(p.value) do
                assert(v <= 0xFF)
                comp:set(i, v)
                i = i+1
            end
        end
    end
end

--
-- COMPUTER TESTS
--

function computer_tests()
    print("#")
    print("# computer")
    print("#")

    local c = computer()
    
    c:set(0x12, 0xAF)
    equals(c:get(0x12), 0xAF, "memory set")

    c:set32(0x0, 0x12345678)
    equals(c:get(0x0), 0x78, "set32 (byte 0)")
    equals(c:get(0x1), 0x56, "set32 (byte 1)")
    equals(c:get(0x2), 0x34, "set32 (byte 2)")
    equals(c:get(0x3), 0x12, "set32 (byte 3)")

    c.offset = 0x1000
    equals(c.offset, 0x1000, "offset")

    c:set(0x12, 0xBB)
    equals(c.physical_memory[0x1012], 0xBB, "offset from physical memory")
    equals(c:get(0x12), 0xBB, "offset from offset")
    c.offset = 0x0
    equals(c:get(0x12), 0xAF, "offset disabled")
end


function cpu_tests()
    print("#")
    print("# cpu")
    print("#")

    function sanity()
        print("# sanity")

        local comp = computer()
        local cpu = comp.cpu[1]

        cpu.A = 0x24
        cpu.B = 0xBF
        equals(cpu.B, 0xBF, "B")
        equals(cpu.A, 0x24, "A")

        equals(cpu.flags.Z, false, "Z")
        cpu.flags.Z = true
        equals(cpu.flags.Z, true, "Z")
        equals(cpu.FL, 0x4, "FL")

        compile(comp, "mov B, C")
        equals(comp:get(0x0), 0x01, "mov B, C [0]")
        equals(comp:get(0x1), 0x12, "mov B, C [1]")
    end

    sanity()
end

--
-- MAIN
--

computer_tests()
cpu_tests()

if err_count == 0 then
    print("\27[32mSUCCESS\27[0m: " .. test_count .. " tests executed successfully.")
else
    print("*** \27[31mFAIL\27[0m: " .. err_count .. " tests failed (from " .. test_count .. " tests executed)")
    os.exit(127)
end
