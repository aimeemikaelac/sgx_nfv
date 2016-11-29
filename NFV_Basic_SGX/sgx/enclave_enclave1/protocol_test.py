#!/usr/bin/python

import socket

if __name__ == "__main__":
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", 10000))
    s.send('data_len:5|data:testy')
    chunk = s.recv(1000)
    print "Received {} bytes".format(chunk)
    print "Received data: {}".format(chunk)