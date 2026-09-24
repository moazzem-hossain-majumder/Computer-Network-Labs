import socket


port = 5050
format = "utf-8"
buffer = 16
disconnected = "End"

hostname = socket.gethostname()
host_ip = socket.gethostbyname(hostname)


server_socket_address = (host_ip, port)


client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(server_socket_address)

def msg_to_be_sent(msg):
    message = msg.encode(format)
    msg_length = len(message)
    msg_length = str(msg_length).encode(format)
    msg_length += b" " * (buffer-len(msg_length))


    client.send(msg_length)
    client.send(message)


    print(client.recv(2048).decode(format))


msg_to_be_sent(f"IP address of the client is {host_ip} and the device name is {hostname}")
msg_to_be_sent(disconnected)
