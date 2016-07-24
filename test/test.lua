luisavm = require('luisavm')

--luisavm.debug_log(true)

--
-- HELPER FUNCTIONS
--

-- {{{

test_count = 0
err_count = 0

function equals(code, expect, text)
    test_count = test_count + 1
    local f, err = load('return (' .. code .. ')')
    if f == nil then
        print("[\27[31merr\27[0m] compilation error: " .. err)
        err_count = err_count + 1
        return
    end
    local ok, tested = pcall(f)
    if not ok then
        print("[\27[31merr\27[0m] error executing " .. text .. " == " .. string.format('0x%X', expect) .. ": " .. tested)
        err_count = err_count + 1
    elseif tested ~= expect then
        print("[\27[31merr\27[0m] " .. text .. " == " .. string.format('0x%X', tested) .. " (expected " .. string.format('0x%X', expect) .. ')')
        err_count = err_count + 1
    else
        print("[\27[32mok\27[0m] " .. text .. " == " .. string.format('0x%X', expect))
    end
end

function computer()
    local computer = luisavm.create_computer(1024 * 1024)
    computer:add_cpu()
    return computer
end

--}}}

--
-- COMPUTER TESTS
--

function computer_tests()
    print('#')
    print('# computer')
    print('#')

    local c = computer()
    
    c:set(0x12, 0xAF)
    equals(c:get(0x12), 0xAF, "memory set")

    c:set32(0x0, 0x12345678)
    equals(c:get(0x0), 0x78, "set32 (byte 0)")
    equals(c:get(0x1), 0x56, "set32 (byte 1)")
    equals(c:get(0x2), 0x34, "set32 (byte 2)")
    equals(c:get(0x3), 0x12, "set32 (byte 3)")

    c.offset = 0x1000
    equals(c.offset, 0x1000, 'offset')

    c:set(0x12, 0xBB)
    equals(c.physical_memory[0x1012], 0xBB, 'offset from physical memory')
    equals(c:get(0x12), 0xBB, 'offset from offset')
    c.offset = 0x0
    equals(c:get(0x12), 0xAF, 'offset disabled')
end


function cpu_tests()
    print('#')
    print('# cpu')
    print('#')

    local cpu = computer().cpu[1]
    cpu.A = 0x24
    cpu.B = 0xBF
    equals(cpu.B, 0xBF, 'B')
    equals(cpu.A, 0x24, 'A')
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
