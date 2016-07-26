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
}  --}}}

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


function run(comp, code)
    last_run = code
    compile(comp, code)
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

        compile(comp, "mov B, C")
        equals(comp:get(0x0), 0x01, "mov B, C [0]")
        equals(comp:get(0x1), 0x12, "mov B, C [1]")
    end


    function mov()
        print("# mov")

        local comp = computer()
        local cpu = comp.cpu[1]

        comp:reset(); cpu.B = 0x42 ; run(comp, "mov A, B")
        equals(cpu.A, 0x42)
        equals(cpu.PC, 3, "checking PC position")

        comp:reset() ; run(comp, "mov A, 0x34") 
        equals(cpu.A, 0x34)
  
        comp:reset() ; run(comp, "mov A, 0x1234") 
        equals(cpu.A, 0x1234)
  
        comp:reset() ; run(comp, "mov A, 0xFABC1234") 
        equals(cpu.A, 0xFABC1234)

        print("# Test movement flags")
  
        comp:reset() ; run(comp, "mov A, 0")
        equals(cpu.Z, true, "cpu.Z = 1")
        equals(cpu.P, true, "cpu.P = 1")
        equals(cpu.S, false, "cpu.S = 0")

        comp:reset() ; run(comp, "mov A, 0xF0000001")
        equals(cpu.Z, false, "cpu.Z = 0")
        equals(cpu.P, false, "cpu.P = 0")
        equals(cpu.S, true, "cpu.S = 1")
    end


    function movb()
        print("# 8-bit movement (movb)")

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
  
        cpu.A = 0xF000 ; comp:set(0xF000, 0x42)
        comp:reset() ; run(comp, "movb [0xCC64], [A]")
        equals(comp:get(0xCC64), 0x42)
  
        comp:set32(0xABF0, 0x1234); comp:set(0x1234, 0x3F);
        comp:reset() ; run(comp, "movb [0x64], [0xABF0]")
        equals(comp:get(0x64), 0x3F)
    end


    function movw()
        print("# 16-bit movement (movw)")
  
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
  
        cpu.A = 0xF000; comp:set16(0xF000, 0x4245); 
        comp:reset() ; run(comp, "movw [0xCC64], [A]")
        equals(comp:get16(0xCC64), 0x4245)
  
        comp:set32(0xABF0, 0x1234); comp:set16(0x1234, 0x3F54);
        comp:reset() ; run(comp, "movw [0x64], [0xABF0]")
        equals(comp:get16(0x64), 0x3F54)
    end


    function movd()
        print("# 32-bit movement (movd)")
  
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
  
        cpu.A = 0xF000; comp:set32(0xF000, 0x4245AABB); 
        comp:reset() ; run(comp, "movd [0xCC64], [A]")
        equals(comp:get32(0xCC64), 0x4245AABB)
  
        comp:set32(0xABF0, 0x1234); comp:set32(0x1234, 0x3F54FABC);
        comp:reset() ; run(comp, "movd [0x64], [0xABF0]")
        equals(comp:get32(0x64), 0x3F54FABC)
    end


    function logic()
        print("# Logic operations")

        comp:reset() ; cpu.A = b(1010); cpu.B = b(1100) ; run(comp, "or A, B")
        equals(cpu.A, b(1110))
        equals(cpu.S, false, "cpu.S == 0")
        equals(cpu.P, true, "cpu.P == 1")
        equals(cpu.Z, false, "cpu.Z == 0")
        equals(cpu.Y, false, "cpu.Y == 0")
        equals(cpu.V, false, "cpu.V == 0")

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
        equals(cpu.Z, true, "cpu.Z == 1")

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
        equals(cpu.A, b(11111111111111111111111100110101))
    end


    function integer_math()
        print("# Integer arithmetic")
  
        comp:reset() ; cpu.A = 0x12; cpu.B = 0x20 ; run(comp, "add A, B")
        equals(cpu.A, 0x32)
  
        comp:reset() ; cpu.A = 0x12 ; run(comp, "add A, 0x20")
        equals(cpu.A, 0x32)

        comp:reset() ; cpu.A = 0x12 ; cpu.Y = true ; run(comp, "add A, 0x20")
        equals(cpu.A, 0x33, "add A, 0x20 (with carry)")

        comp:reset() ; cpu.A = 0x12 ; run(comp, "add A, 0x2000")
        equals(cpu.A, 0x2012)

        comp:reset() ; cpu.A = 0x10000012 ; run(comp, "add A, 0xF0000000")
        equals(cpu.A, 0x12)
        equals(cpu.Y, true, "cpu.Y == 1")

        comp:reset() ; cpu.A = 0x30; cpu.B = 0x20 ; run(comp, "sub A, B")
        equals(cpu.A, 0x10)
        equals(cpu.S, false, "cpu.S == 0")

        comp:reset() ; cpu.A = 0x20; cpu.B = 0x30 ; run(comp, "sub A, B")
        equals(cpu.A, 0xFFFFFFF0, "sub A, B (negative)")
        equals(cpu.S, true, "cpu.S == 1")

        comp:reset() ; cpu.A = 0x22 ; run(comp, "sub A, 0x20")
        equals(cpu.A, 0x2)

        comp:reset() ; cpu.A = 0x22; cpu.Y = true ; run(comp, "sub A, 0x20")
        equals(cpu.A, 0x1, "sub A, 0x20 (with carry)")

        comp:reset() ; cpu.A = 0x12 ; run(comp, "sub A, 0x2000")
        equals(cpu.A, 0xFFFFE012)
        equals(cpu.S, true, "cpu.S == 1")
        equals(cpu.Y, true, "cpu.Y == 1")

        comp:reset() ; cpu.A = 0x10000012 ; run(comp, "sub A, 0xF0000000")
        equals(cpu.A, 0x20000012)
        equals(cpu.Y, true, "cpu.Y == 1")

        comp:reset() ; run(comp, "cmp A, B")
        equals(cpu.Z, true)

        comp:reset() ; run(comp, "cmp A, 0x12")
        equals(cpu.LT and not cpu.GT, true)

        comp:reset() ; cpu.A = 0x6000 ; run(comp, "cmp A, 0x1234")
        equals(not cpu.LT and cpu.GT, true)

        comp:reset() ; cpu.A = 0xF0000000 ; run(comp, "cmp A, 0x12345678")
        equals(not cpu.LT and cpu.GT, true)  -- because of the signal!

        comp:reset() ; cpu.A = 0x0 ; run(comp, "cmp A")
        equals(cpu.Z, true)

        comp:reset() ; cpu.A = 0xF0; cpu.B = 0xF000 ; run(comp, "mul A, B")
        equals(cpu.A, 0xE10000)

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mul A, 0x12")
        equals(cpu.A, 0x147A8)

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mul A, 0x12AF")
        equals(cpu.A, 0x154198C)
        equals(cpu.V, false, "cpu.V == 0")

        comp:reset() ; cpu.A = 0x1234 ; run(comp, "mul A, 0x12AF87AB")
        equals(cpu.A, 0x233194BC)
        equals(cpu.V, true, "cpu.V == 1")

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
        equals(cpu.Z, true, "cpu.Z == 1")

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
        equals(cpu.Y, true, "cpu.Y == 1")
        equals(cpu.Z, true, "cpu.Z == 1")

        comp:reset() ; cpu.A = 0x42 ; run(comp, "dec A")
        equals(cpu.A, 0x41)

        comp:reset() ; cpu.A = 0x0 ; run(comp, "dec A")
        equals(cpu.A, 0xFFFFFFFF, "dec A (underflow)")
        equals(cpu.Z, false, "cpu.Z == 0")
    end


    function branches()
        print("# Branch operations")

        comp:reset() ; cpu.Z = true; cpu.A = 0x1000 ; run(comp, "bz A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bz A")
        equals(cpu.PC, 0x2, "bz A (false)")

        comp:reset() ; cpu.Z = true ; run(comp, "bz 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bnz A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; run(comp, "bnz 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.S = true; cpu.A = 0x1000 ; run(comp, "bneg A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bneg A")
        equals(cpu.PC, 0x2, "bneg A (false)")

        comp:reset() ; cpu.S = true ; run(comp, "bneg 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; cpu.A = 0x1000 ; run(comp, "bpos A")
        equals(cpu.PC, 0x1000)

        comp:reset() ; run(comp, "bpos 0x1000")
        equals(cpu.PC, 0x1000)

        comp:reset() ; run(comp, "jmp 0x12345678")
        equals(cpu.PC, 0x12345678)
    end


    --[[
    function stack()
        print("# Stack operations")

  mb.reset();
  cpu.SP = 0xFFF; 
  cpu.A = 0xABCDEF12;

  comp:setArray(0x0, Debugger.encode('pushb A'));
  comp:setArray(0x2, Debugger.encode('pushb 0x12'));
  comp:setArray(0x4, Debugger.encode('pushw A'));
  comp:setArray(0x6, Debugger.encode('pushd A'));

  comp:setArray(0x8, Debugger.encode('popd B'));
  comp:setArray(0xA, Debugger.encode('popw B'));
  comp:setArray(0xC, Debugger.encode('popb B'));

  comp:setArray(0xE, Debugger.encode('popx 1'));

  mb.step();
        equals(comp:get(0xFFF), 0x12, "pushb A")
        equals(cpu.SP, 0xFFE, "SP = 0xFFE")

  mb.step();
        equals(comp:get(0xFFE), 0x12, "pushb 0x12")
        equals(cpu.SP, 0xFFD, "SP = 0xFFD")

  mb.step();
        equals(comp:get16(0xFFC), 0xEF12)
        equals(comp:get(0xFFD), 0xEF)
        equals(comp:get(0xFFC), 0x12)
        equals(cpu.SP, 0xFFB, "SP = 0xFFB")

  mb.step();
  t.equal(comp:get32(0xFF8), 0xABCDEF12);
        equals(cpu.SP, 0xFF7, "SP = 0xFF7")

  mb.step();
        equals(cpu.B, 0xABCDEF12, "popd B")

  mb.step();
        equals(cpu.B, 0xEF12, "popw B")

  mb.step();
        equals(cpu.B, 0x12, "popb B")

  mb.step();
        equals(cpu.SP, 0xFFF, "popx 1")

  // all registers
  s = opc('push.a', () => {
    cpu.SP = 0xFFF;
    cpu.A = 0xA1B2C3E4;
    cpu.B = 0xFFFFFFFF;
  });
        equals(cpu.SP, 0xFCF)
        equals(comp:get32(0xFFC), 0xA1B2C3E4, "A is saved")
        equals(comp:get32(0xFF8), 0xFFFFFFFF, "B is saved")
  
  s = opc('pop.a', () => {
    cpu.SP = 0xFCF;
    comp:set32(0xFFC, 0xA1B2C3E4);
    comp:set32(0xFF8, 0xFFFFFFFF);
  });
        equals(cpu.SP, 0xFFF)
        equals(cpu.A, 0xA1B2C3E4, "A is restored")
        equals(cpu.B, 0xFFFFFFFF, "B is restored")
    end

  // others
        print("# Others")

  opc('nop');
  
        comp:reset() ; run(comp, "dbg")
        equals(cpu.activateDebugger, true)

        comp:reset() ; run(comp, "halt")
        equals(cpu.systemHalted, true)

  s = opc('swap A, B', () => {
    cpu.A = 0xA;
    cpu.B = 0xB;
  });
        equals(cpu.A == 0xB and cpu.B == 0xA, true)

  t.end();

});


test('CPU: subroutines and system calls', t => {

  let [mb, cpu] = makeCPU();

  // jsr
  mb.reset();
  comp:setArray(0x200, Debugger.encode('jsr 0x1234'));
  comp:setArray(0x1234, Debugger.encode('ret'));
  cpu.PC = 0x200;
  cpu.SP = 0xFFF;
  mb.step();
        equals(cpu.PC, 0x1234, "jsr 0x1234")
        equals(comp:get(0xFFC), 0x5, "[FFC] = 0x5")
        equals(comp:get(0xFFD), 0x2, "[FFD] = 0x2")
        equals(cpu.SP, 0xFFB, "SP = 0xFFB")
        equals(comp:get32(0xFFC), 0x200 + 5, "address in stack") 

  mb.step();
        equals(cpu.PC, 0x205, "ret")
        equals(cpu.SP, 0xFFF, "SP = 0xFFF")

  // sys
  mb.reset();
  cpu.SP = 0xFFF;
  comp:setArray(0, Debugger.encode('sys 2'));
  comp:set32(cpu.CPU_SYSCALL_VECT + 8, 0x1000);
        equals(cpu._syscallVector[2], 0x1000, "syscall vector")
  comp:setArray(0x1000, Debugger.encode('sret'));

  mb.step();
        equals(cpu.PC, 0x1000, "sys 2")
        equals(cpu.SP, 0xFFB, "SP = 0xFFD")
  mb.step();
        equals(cpu.PC, 0x2, "sret")
        equals(cpu.SP, 0xFFF, "SP = 0xFFF")

  t.end();
--]]

    sanity()
    mov()
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
