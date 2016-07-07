require 'socket'
require 'test/unit'



class TestLillput < Test::Unit::TestCase

    def setup
        #spawn('./lilliput -D')
        tries = 10
        begin
            @s = TCPSocket.open('localhost', 5999)
        rescue
            sleep(0.3)
            retry unless (tries -= 1).zero?
        end
        @s.gets # ignore welcome message
    end

    def get_i(s)
        @s.getc
        @s.getc
        @s.puts(s) ; @s.flush
        return @s.gets.to_i(16)
    end

    def test_memory
        @s.puts("m w 0x12 0xAF")
        assert_equal(get_i("m r 0x12"), 0xAF)
    end

    def teardown
        @s.puts 'q'
        sleep(1)
    end

end
