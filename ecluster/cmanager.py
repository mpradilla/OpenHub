import socket

HOST = 'master-hpc-mox.uniandes.edu.co'    # The remote host
PORT = 50000                               # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
text = 'Hello from python. Here I will send you a DSM matrix'
s.sendall(text.encode('utf8'))
data = s.recv(1024)
s.close()

print 'Received', repr(data)