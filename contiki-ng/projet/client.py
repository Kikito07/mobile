import socket

msgFromClient       = "Hello UDP Server"

bytesToSend         = str.encode(msgFromClient)

serverAddressPort   = ("bbbb::c30c:0:0:1", 3000)

bufferSize          = 1024


UDPClientSocket = socket.socket(family=socket.AF_INET6, type=socket.SOCK_DGRAM)

UDPClientSocket.sendto(bytesToSend, serverAddressPort)