luisavm = require("luisavm")

--luisavm.debug_log(true)

--
-- HELPER FUNCTIONS
--

-- {{{

test_count = 0
err_count = 0
last_run = nil

function equals(tested, expect, text)
    text = text or last_run
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

function b(value)
    local n = 1
    local ret = 0
    while value > 0 do
        if (value & 1) ~= 0 then
            ret = ret + n
        end
        n = n*2
        value = math.floor(value / 10)
    end
    return ret
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

-- {{{

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

function compile(comp, code)
    local ret = {}
    local registers = { A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7, I=8, J=9, K=10, L=11, FP=12, SP=13, PC=14, FL=15 }

    -- parse parameter
    function parse_parameter(par)
        local fst = par:sub(1, 1)
        if fst == '[' then
            if par:sub(2, 2):match('%a') then
                return { type='regind', value={ assert(registers[par:sub(2, #par-1)]) } }
            else
                local v = math.tointeger(par:sub(2, #par-1))
                return { type='indv32', value={ (v & 0xFF), ((v >> 8) & 0xFF), ((v >> 16) & 0xFF), ((v >> 24) & 0xFF) } }
            end
        elseif fst:match('%a') then
            return { type='reg', value={ assert(registers[par]) } }
        else
            local v = assert(math.tointeger(par))
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
    local instruction, parameters = code:match("([%w%.]+)%s?.*"), {}
    for w in code:gmatch("[%s,]([%w%[%]]+)") do
        parameters[#parameters+1] = parse_parameter(w)
    end
    assert(instruction)

    -- upgrade parameter (HACK)
    if instruction:sub(1,1) == 'b' or instruction:sub(1,1) == 'j' then  -- is a branch
        if parameters[1].type == 'v8' or parameters[1].type == 'v16' then
            parameters[1].type = 'v32'
        end
    end

    -- find operator
    for opcode, op in pairs(opcodes) do
        if op.instruction == instruction and #op.parameters == #parameters then
            local eq = true
            for i,p in ipairs(op.parameters) do
                if p ~= parameters[i].type then eq = false end
            end
            if eq then
                ret[#ret+1] = opcode
                goto found
            end
        end
    end
    assert(false, "opcode from code '" .. code .. "' not found")
::found::
    
    -- add parameters
    if (parameters[1] and parameters[1].type:sub(1, 3) == 'reg') and (parameters[2] and parameters[2].type:sub(1, 3) == 'reg') then
        ret[#ret+1] = (parameters[1].value[1] << 4) | parameters[2].value[1]
    else
        local i = 1
        for _,p in ipairs(parameters) do
            for _,v in ipairs(p.value) do
                assert(v <= 0xFF)
                ret[#ret+1] = v
                i = i+1
            end
        end
    end

    return ret
end


function run(comp, code)
    last_run = code
    local bytes = compile(comp, code)
    for i,b in ipairs(bytes) do
        comp:set(i-1, b)
    end
    comp:step()
end


-- }}}

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

        local bytes = compile(comp, "mov B, C")
        equals(bytes[1], 0x01, "mov B, C [0]")
        equals(bytes[2], 0x12, "mov B, C [1]")
    end


    function mov()
        print("# mov")

        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset(); cpu.B = 0x42 ; run(comp, "mov A, B")
        equals(cpu.A, 0x42)
        equals(cpu.PC, 2, "checking PC position")

        comp:reset() ; run(comp, "mov A, 0x34") 
        equals(cpu.A, 0x34)
  
        comp:reset() ; run(comp, "mov A, 0x1234") 
        equals(cpu.A, 0x1234)
  
        comp:reset() ; run(comp, "mov A, 0xFABC1234") 
        equals(cpu.A, 0xFABC1234)

        print("# Test movement flags")
  
        comp:reset() ; run(comp, "mov A, 0")
        equals(cpu.flags.Z, true, "cpu.flags.Z")
        equals(cpu.flags.S, false, "cpu.flags.S")

        comp:reset() ; run(comp, "mov A, 0xF0000001")
        equals(cpu.flags.Z, false, "cpu.flags.Z")
        equals(cpu.flags.S, true, "cpu.flags.S")
    end


    function movb()
        print("# 8-bit movement (movb)")

        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; cpu.B = 0x1000; comp:set(cpu.B, 0xAB) ; run(comp, "movb A, [B]") 
        equals(cpu.A, 0xAB)
  
        comp:reset() ; comp:set(0x1000, 0xAB) ; run(comp, "movb A, [0x1000]")
        equals(cpu.A, 0xAB)

        comp:reset() ; cpu.A = 0x64 ; cpu.C = 0x32 ; run(comp, "movb [C], A")
        equals(comp:get(0x32), 0x64)

        comp:reset() ; cpu.A = 0x64 ; run(comp, "movb [A], 0xFA")
        equals(comp:get(0x64), 0xFA)

        comp:reset() ; cpu.A = 0x32 ; cpu.B = 0x64 ; comp:set(0x64, 0xFF) ; run(comp, "movb [A], [B]")
        equals(comp:get(0x32), 0xFF)

        comp:reset() ; cpu.A = 0x32; comp:set(0x6420, 0xFF) ; run(comp, "movb [A], [0x6420]")
        equals(comp:get(0x32), 0xFF)

        comp:reset() ; cpu.A = 0xAC32 ; run(comp, "movb [0x64], A")
        equals(comp:get(0x64), 0x32)

        comp:reset() ; run(comp, "movb [0x64], 0xF0")
        equals(comp:get(0x64), 0xF0)
  
        comp:reset() ; cpu.A = 0xF000 ; comp:set(0xF000, 0x42)
        run(comp, "movb [0xCC64], [A]")
        equals(comp:get(0xCC64), 0x42)
  
        comp:reset() ; comp:set32(0xABF0, 0x3F)
        run(comp, "movb [0x64], [0xABF0]")
        equals(comp:get(0x64), 0x3F)
    end


    function movw()
        print("# 16-bit movement (movw)")
  
        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; cpu.B = 0x1000 ; comp:set16(cpu.B, 0xABCD) ; run(comp, "movw A, [B]") 
        equals(cpu.A, 0xABCD)
  
        comp:reset() ; comp:set16(0x1000, 0xABCD) ; run(comp, "movw A, [0x1000]")
        equals(cpu.A, 0xABCD)

        comp:reset() ; cpu.A = 0x6402 ; run(comp, "movw [A], A")
        equals(comp:get16(0x6402), 0x6402)

        comp:reset() ; cpu.A = 0x64 ; run(comp, "movw [A], 0xFABA")
        equals(comp:get16(0x64), 0xFABA)

        comp:reset() ; cpu.A = 0x32CC ; cpu.B = 0x64 ; comp:set16(0x64, 0xFFAB) ; run(comp, "movw [A], [B]")
        equals(comp:get16(0x32CC), 0xFFAB)

        comp:reset() ; cpu.A = 0x32; comp:set16(0x6420, 0xFFAC) ; run(comp, "movw [A], [0x6420]")
        equals(comp:get16(0x32), 0xFFAC)

        comp:reset() ; cpu.A = 0xAB32AC ; run(comp, "movw [0x64], A")
        equals(comp:get16(0x64), 0x32AC)

        comp:reset() ; run(comp, "movw [0x64], 0xF0FA")
        equals(comp:get16(0x64), 0xF0FA)
  
        comp:reset() ; cpu.A = 0xF000; comp:set16(0xF000, 0x4245); 
        run(comp, "movw [0xCC64], [A]")
        equals(comp:get16(0xCC64), 0x4245)
  
        comp:reset() ; comp:set16(0xABF0, 0x3F54)
        run(comp, "movw [0x64], [0xABF0]")
        equals(comp:get16(0x64), 0x3F54)
    end


    function movd()
        print("# 32-bit movement (movd)")
  
        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; cpu.B = 0x1000; comp:set32(cpu.B, 0xABCDEF01) ; run(comp, "movd A, [B]") 
        equals(cpu.A, 0xABCDEF01)
  
        comp:reset() ; comp:set32(0x1000, 0xABCDEF01) ; run(comp, "movd A, [0x1000]")
        equals(cpu.A, 0xABCDEF01)

        comp:reset() ; cpu.A = 0x16402 ; run(comp, "movd [A], A")
        equals(comp:get32(0x16402), 0x16402)

        comp:reset() ; cpu.A = 0x64 ; run(comp, "movd [A], 0xFABA1122")
        equals(comp:get32(0x64), 0xFABA1122)

        comp:reset() ; cpu.A = 0x32CC; cpu.B = 0x64; comp:set32(0x64, 0xFFAB5678) ; run(comp, "movd [A], [B]")
        equals(comp:get32(0x32CC), 0xFFAB5678)

        comp:reset() ; cpu.A = 0x32; comp:set32(0x6420, 0xFFAC9876) ; run(comp, "movd [A], [0x6420]")
        equals(comp:get32(0x32), 0xFFAC9876)

        comp:reset() ; cpu.A = 0xAB32AC44 ; run(comp, "movd [0x64], A")
        equals(comp:get32(0x64), 0xAB32AC44)

        comp:reset() ; run(comp, "movd [0x64], 0xF0FA1234")
        equals(comp:get32(0x64), 0xF0FA1234)
  
        comp:reset() ; cpu.A = 0xF000; comp:set32(0xF000, 0x4245AABB); 
        run(comp, "movd [0xCC64], [A]")
        equals(comp:get32(0xCC64), 0x4245AABB)
  
        comp:reset() ; comp:set32(0xABF0, 0x3F54FABC)
        run(comp, "movd [0x64], [0xABF0]")
        equals(comp:get32(0x64), 0x3F54FABC)
    end


    function swap()
        print("# register swap")

        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; cpu.A = 0xA ; cpu.B = 0xB ; run(comp, "swap A, B")
        equals(cpu.A == 0xB and cpu.B == 0xA, true)
    end


    function logic()
        print("# Logic operations")

        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; cpu.A = b(1010); cpu.B = b(1100) ; run(comp, "or A, B")
        equals(cpu.A, b(1110))
        equals(cpu.flags.S, false, "cpu.flags.S")
        equals(cpu.flags.Z, false, "cpu.flags.Z")
        equals(cpu.flags.Y, false, "cpu.flags.Y")
        equals(cpu.flags.V, false, "cpu.flags.V")

        comp:reset() ; cpu.A = b(11) ; run(comp, "or A, 0x4")
        equals(cpu.A, b(111))

        comp:reset() ; cpu.A = b(111) ; run(comp, "or A, 0x4000")
        equals(cpu.A, 0x4007)

        comp:reset() ; cpu.A = 0x10800000 ; run(comp, "or A, 0x2A426653")
        equals(cpu.A, 0x3AC26653)

        comp:reset() ; cpu.A = b(1010); cpu.B = b(1100) ; run(comp, "xor A, B")
        equals(cpu.A, b(110))

        comp:reset() ; cpu.A = b(11) ; run(comp, "xor A, 0x4")
        equals(cpu.A, b(111))

        comp:reset() ; cpu.A = 0xFF0 ; run(comp, "xor A, 0xFF00")
        equals(cpu.A, 0xF0F0)

        comp:reset() ; cpu.A = 0x148ABD12 ; run(comp, "xor A, 0x2A426653")
        equals(cpu.A, 0x3EC8DB41)

        comp:reset() ; cpu.A = b(11); cpu.B = b(1100) ; run(comp, "and A, B")
        equals(cpu.A, 0)
        equals(cpu.flags.Z, true, "cpu.flags.Z")

        comp:reset() ; cpu.A = b(11) ; run(comp, "and A, 0x7")
        equals(cpu.A, b(11))

        comp:reset() ; cpu.A = 0xFF0 ; run(comp, "and A, 0xFF00")
        equals(cpu.A, 0xF00)

        comp:reset() ; cpu.A = 0x148ABD12 ; run(comp, "and A, 0x2A426653")
        equals(cpu.A, 0x22412)

        comp:reset() ; cpu.A = b(10101010); cpu.B = 4 ; run(comp, "shl A, B")
        equals(cpu.A, b(101010100000))

        comp:reset() ; cpu.A = b(10101010) ; run(comp, "shl A, 4")
        equals(cpu.A, b(101010100000))

        comp:reset() ; cpu.A = b(10101010); cpu.B = 4 ; run(comp, "shr A, B")
        equals(cpu.A, b(1010))

        comp:reset() ; cpu.A = b(10101010) ; run(comp, "shr A, 4")
        equals(cpu.A, b(1010))

        comp:reset() ; cpu.A = b(11001010) ; run(comp, "not A")
        equals(cpu.A, 0xFFFFFF35)
    end


    function integer_math()
        print("# Integer arithmetic")
  
        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; cpu.A = 0x12; cpu.B = 0x20 ; run(comp, "add A, B")
        equals(cpu.A, 0x32)
  
        comp:reset() ; cpu.A = 0x12 ; run(comp, "add A, 0x20")
        equals(cpu.A, 0x32)

        comp:reset() ; cpu.A = 0x12 ; cpu.flags.Y = true ; run(comp, "add A, 0x20")
        equals(cpu.A, 0x33, "add A, 0x20 (with carry)")

        comp:reset() ; cpu.A = 0x12 ; run(comp, "add A, 0x2000")
        equals(cpu.A, 0x2012)

        comp:reset() ; cpu.A = 0x10000012 ; run(comp, "add A, 0xF0000000")
        equals(cpu.A, 0x12)
        equals(cpu.flags.Y, true, "cpu.flags.Y")

        comp:reset() ; cpu.A = 0x30; cpu.B = 0x20 ; run(comp, "sub A, B")
        equals(cpu.A, 0x10)
        equals(cpu.flags.S, false, "cpu.flags.S")

        comp:reset() ; cpu.A = 0x20; cpu.B = 0x30 ; run(comp, "sub A, B")
        equals(cpu.A, 0xFFFFFFF0, "sub A, B (negative)")
        equals(cpu.flags.S, true, "cpu.flags.S")

        comp:reset() ; cpu.A = 0x22 ; run(comp, "sub A, 0x20")
        equals(cpu.A, 0x2)

        comp:reset() ; cpu.A = 0x22; cpu.flags.Y = true ; run(comp, "sub A, 0x20")
        equals(cpu.A, 0x1, "sub A, 0x20 (with carry)")

        comp:reset() ; cpu.A = 0x12 ; run(comp, "sub A, 0x2000")
        equals(cpu.A, 0xFFFFE012)
        equals(cpu.flags.S, true, "cpu.flags.S")
        equals(cpu.flags.Y, true, "cpu.flags.Y")

        comp:reset() ; cpu.A = 0x10000012 ; run(comp, "sub A, 0xF0000000")
        equals(cpu.A, 0x20000012)
        equals(cpu.flags.Y, true, "cpu.flags.Y")

        comp:reset() ; run(comp, "cmp A, B")
        equals(cpu.flags.Z, true)

        comp:reset() ; run(comp, "cmp A, 0x12")
        equals(cpu.flags.LT and not cpu.flags.GT, true)

        comp:reset() ; cpu.A = 0x6000 ; run(comp, "cmp A, 0x1234")
        equals(not cpu.flags.LT and cpu.flags.GT, true)

        comp:reset() ; cpu.A = 0xF0000000 ; run(comp, "cmp A, 0x12345678")
        equals(not cpu.flags.LT and cpu.flags.GT, true)  -- because of the signal!

        comp:reset() ; cpu.A = 0x0 ; run(comp, "cmp A")
        equals(cpu.flags.Z, true)

        comp:reset() ; cpu.A = 0xF0; cpu.B = 0xF000 ; run(comp, "mul A, B")
        equals(cpu.A, 0xE10000)

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mul A, 0x12")
        equals(cpu.A, 0x147A8)

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mul A, 0x12AF")
        equals(cpu.A, 0x154198C)
        equals(cpu.flags.V, false, "cpu.flags.V")

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mul A, 0x12AF87AB")
        equals(cpu.A, 0x233194BC)
        equals(cpu.flags.V, true, "cpu.flags.V")

        comp:reset() ; cpu.A = 0xF000; cpu.B = 0xF0 ; run(comp, "idiv A, B")
        equals(cpu.A, 0x100)

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "idiv A, 0x12")
        equals(cpu.A, 0x102)

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "idiv A, 0x2AF")
        equals(cpu.A, 0x6)

        comp:reset() ; cpu.A = 0x123487AB ; run(comp, "idiv A, 0x12AF")
        equals(cpu.A, 0xF971)

        comp:reset() ; cpu.A = 0xF000; cpu.B = 0xF0 ; run(comp, "mod A, B")
        equals(cpu.A, 0x0)
        equals(cpu.flags.Z, true, "cpu.flags.Z")

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mod A, 0x12")
        equals(cpu.A, 0x10)

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mod A, 0x2AF")
        equals(cpu.A, 0x21A)

        comp:reset() ; cpu.A = 0x123487AB ; run(comp, "mod A, 0x12AF")
        equals(cpu.A, 0x116C)

        comp:reset() ; cpu.A = 0x42 ; run(comp, "inc A")
        equals(cpu.A, 0x43)

        comp:reset() ; cpu.A = 0xFFFFFFFF ; run(comp, "inc A")
        equals(cpu.A, 0x0, "inc A (overflow)")
        equals(cpu.flags.Y, true, "cpu.flags.Y")
        equals(cpu.flags.Z, true, "cpu.flags.Z")

        comp:reset() ; cpu.A = 0x42 ; run(comp, "dec A")
        equals(cpu.A, 0x41)

        comp:reset() ; cpu.A = 0x0 ; run(comp, "dec A")
        equals(cpu.A, 0xFFFFFFFF, "dec A (underflow)")
        equals(cpu.flags.Z, false, "cpu.flags.Z")
    end


    function branches()
        print("# Branch operations")

        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; cpu.flags.Z = true; cpu.A = 0x1000 ; run(comp, "bz A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bz A")
        equals(cpu.PC, 0x2, "bz A (false)")

        comp:reset() ; cpu.flags.Z = true ; run(comp, "bz 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bnz A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; run(comp, "bnz 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.flags.S = true; cpu.A = 0x1000 ; run(comp, "bneg A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bneg A")
        equals(cpu.PC, 0x2, "bneg A (false)")

        comp:reset() ; cpu.flags.S = true ; run(comp, "bneg 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bpos A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; run(comp, "bpos 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; run(comp, "jmp 0x12345678")
        equals(cpu.PC, 0x12345678)
    end


    function stack()
        print("# Stack operations")
        
        local comp = computer()
        local cpu = comp.cpu[1]

        cpu.SP = 0xFFF
        cpu.A = 0xABCDEF12

        for i,b in ipairs(compile(code, "pushb A")) do comp:set((i-1) + 0x0, b) end
        for i,b in ipairs(compile(code, "pushb 0x12")) do comp:set((i-1) + 0x2, b) end
        for i,b in ipairs(compile(code, "pushw A")) do comp:set((i-1) + 0x4, b) end
        for i,b in ipairs(compile(code, "pushd A")) do comp:set((i-1) + 0x6, b) end

        for i,b in ipairs(compile(code, "popd B")) do comp:set((i-1) + 0x8, b) end
        for i,b in ipairs(compile(code, "popw B")) do comp:set((i-1) + 0xA, b) end
        for i,b in ipairs(compile(code, "popb B")) do comp:set((i-1) + 0xC, b) end

        for i,b in ipairs(compile(code, "popx 1")) do comp:set((i-1) + 0xE, b) end

        comp:step()
        equals(comp:get(0xFFF), 0x12, "pushb A")
        equals(cpu.PC, 0x2, "PC")
        equals(cpu.SP, 0xFFE, "SP")

        comp:step()
        equals(comp:get(0xFFE), 0x12, "pushb 0x12")
        equals(cpu.SP, 0xFFD, "SP")

        comp:step()
        equals(comp:get16(0xFFC), 0xEF12)
        equals(comp:get(0xFFD), 0xEF)
        equals(comp:get(0xFFC), 0x12)
        equals(cpu.SP, 0xFFB, "SP")

        comp:step()
        equals(comp:get32(0xFF8), 0xABCDEF12);
        equals(cpu.SP, 0xFF7, "SP")

        comp:step()
        equals(cpu.B, 0xABCDEF12, "popd B")

        comp:step()
        equals(cpu.B, 0xEF12, "popw B")

        comp:step()
        equals(cpu.B, 0x12, "popb B")

        comp:step()
        equals(cpu.SP, 0xFFF, "popx 1")

        -- all registers
        comp:reset()
        cpu.SP = 0xFFF
        cpu.A = 0xA1B2C3E4
        cpu.B = 0xFFFFFFFF
        run(comp, "push.a")
        equals(cpu.SP, 0xFCF)
        equals(comp:get32(0xFFC), 0xA1B2C3E4, "A is saved")
        equals(comp:get32(0xFF8), 0xFFFFFFFF, "B is saved")
  
        comp:reset()
        cpu.SP = 0xFCF
        comp:set32(0xFFC, 0xA1B2C3E4)
        comp:set32(0xFF8, 0xFFFFFFFF)
        run(comp, "pop.a")
        equals(cpu.SP, 0xFFF)
        equals(cpu.A, 0xA1B2C3E4, "A is restored")
        equals(cpu.B, 0xFFFFFFFF, "B is restored")
    end


    function others()
        print("# Others")

        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset() ; run(comp, "nop")
        equals(cpu.PC, 1, "nop; PC")
  
        -- TODO
        --comp:reset() ; run(comp, "dbg")
        --equals(cpu.activateDebugger, true)

        --comp:reset() ; run(comp, "halt")
        --equals(cpu.systemHalted, true)
    end

    
    function subroutines()
        print("# Subroutines")

        local comp = computer()
        local cpu = comp.cpu[1]

        for i,b in ipairs(compile(code, "jsr 0x1234")) do comp:set((i-1) + 0x200, b) end
        for i,b in ipairs(compile(code, "ret")) do comp:set((i-1) + 0x1234, b) end
        cpu.PC = 0x200
        cpu.SP = 0xFFF
        comp:step()
        equals(cpu.PC, 0x1234, "jsr 0x1234")
        equals(comp:get(0xFFC), 0x5, "[FFC]")
        equals(comp:get(0xFFD), 0x2, "[FFD]")
        equals(cpu.SP, 0xFFB, "SP")
        equals(comp:get32(0xFFC), 0x200 + 5, "address in stack") 

        comp:step()
        equals(cpu.PC, 0x205, "ret")
        equals(cpu.SP, 0xFFF, "SP")
    end


    function breakpoints()
        print("# breakpoints")

        local comp = computer()
        local cpu = comp.cpu[1]

        cpu:add_breakpoint(0xFF)
        equals(cpu:is_breakpoint(0x00), false, "breakpoint in 0x00")
        equals(cpu:is_breakpoint(0xFF), true, "breakpoint in 0xFF (added)")

        cpu:remove_breakpoint(0xFF)
        equals(cpu:is_breakpoint(0xFF), false, "breakpoint in 0xFF (removed)")

        run(comp, "dbg")
        equals(cpu:is_breakpoint(0x1), true)
    end

    -- TODO - interrupt

    sanity()
    mov()
    movb()
    movw()
    movd()
    swap()
    logic()
    integer_math()
    branches()
    stack()
    others()
    subroutines()
    breakpoints()

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
