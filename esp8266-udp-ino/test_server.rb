require 'socket'

udp_socket = UDPSocket.new
udp_socket.bind("localhost", 32001)
while true do
    puts udp_socket.recvfrom(15)
end
