require 'socket'
#TODO: weather forecast

srand();

def RandomMac()
    hex_list = []
    hex_list << 0 << 0
    for i in (2..5)
	hex_list << rand(255)
    end
    return hex_list;
end

mac = RandomMac()

address = "192.168.0.171"
port = 32001
udp_socket = UDPSocket.new

#puts mac

while true do
    d = rand(0..1)
    h = rand(0..100)
    t = (rand(-400..1250)/10.0)
    m = mac.pack("C6")
    m += [d,h,t].pack("CFF")
    puts "#{mac}: #{d}, #{h}, #{t}"
    udp_socket.send(m, 0, address, port)
    sleep(10)
end