require 'socket'
#TODO: weather forecast

address = "192.168.0.171"
port = 32001
udp_socket = UDPSocket.new

while true do
    d = rand(0..1)
    h = rand(0..100)
    t = (rand(-400..1250)/10.0)
    m = [d,h,t].pack("CFF")
    puts "#{m}: #{d}, #{h}, #{t}"
    udp_socket.send(m, 0, address, port)
    sleep(10)
end