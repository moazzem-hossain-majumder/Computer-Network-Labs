import socket


port = 5050
buffer = 16
format = "utf-8"
disconnected = "End"

hostname = socket.gethostname()
host_ip = socket.gethostbyname(hostname)


server_socket_address = (host_ip, port)


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(server_socket_address)


server.listen()
print("Server is listening")

while True:
    conn, addr = server.accept()
    print("Connected to", addr)
    connected = True


    while connected:
        message_length = conn.recv(buffer).decode(format)
        print("Length of the message is", message_length)


        if message_length:
            message_length = int(message_length)
            msg = conn.recv(message_length).decode(format)
            if msg == disconnected:
                conn.send("Goodbye. It was nice to serve you.".encode(format))
                print("Terminating connection with", addr)
                connected = False
            else:
                print(msg)
                conn.send("I have received your message".encode(format))
    conn.close()
