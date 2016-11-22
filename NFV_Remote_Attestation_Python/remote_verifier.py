import socket
import struct
import array
# need to install the backported enum for this
from enum import Enum
from service_provider_util import *

request_struct_size = (8 + 32 + 3*8)/8

IS_SP_REGISTERED = False
GLOBAL_SP_EXTENDED_EPID_GROUP = SP_Extended_EPID_Group()
GLOBAL_AUTHENTICATION_TOKEN = 0
GLOBAL_SP_CREDENTIALS = 0

GLOBAL_CLIENT_PUBLIC_KEY = None

ECP_KEY_SIZE = 256/8

def ra_proc_msg0_req(request_body):
    '''
    Sample processing of SGX msg0    . Since enrollment is not implemented, just return 0
    for most things
    '''
    extended_epid_group_id = struct.unpack('I', request_body)
    if not IS_SP_REGISTERED or extended_epid_group_id is not GLOBAL_SP_EXTENDED_EPID_GROUP:
        ret, GLOBAL_AUTHENTICATION_TOKEN = GLOBAL_SP_EXTENDED_EPID_GROUP.enroll(GLOBAL_SP_CREDENTIALS, GLOBAL_AUTHENTICATION_TOKEN)
        if ret is not 0:
            return -1
        else:
            IS_SP_REGISTERED = True
            return 0
    else:
        return 0

def ra_proc_msg1_req(request_body):
    if not IS_SP_REGISTERED:
        return -1
    ga_x, ga_y, gid = struct.unpack('{}s{}s4s', request_body)
    ret, sig_rl = GLOBAL_SP_EXTENDED_EPID_GROUP.get_sigrl(gid)
    if ret is not 0:
        return -1
    
    GLOBAL_CLIENT_PUBLIC_KEY = ECCDH_Public_Key(ga_x, ga_y)
    
    
    return 0, RA_SAMP_RESPONSE()

def ra_proc_msg3_req(request_body):
    return 0



def handle_connection(clientsocket):
#TODO: make into a thread
    received_request_total = ''
    recv_bytes_total = 0
    recv_bytes = 0
    while True:
        chunk = clientsocket.recv(1000)
        if chunk == '':
            print "Receive error"
        received_request_total = received_request_total + chunk
        recv_bytes_total = recv_bytes_total + len(chunk)
        message_type, size, align, body = struct.unpack('BI3Bs', received_request_total)
        if recv_bytes_total >= (size + request_struct_size):
            message_type, size, align, body = struct.unpack('BI3B{}s'.format(size), received_request_total)
            break

        if recv_bytes_total >= 10000:
            print "Receiving too much data. Aborting"
            return -1

    if message_type == RA_MSG_TYPE.TYPE_RA_MSG0:
        ret, response = ra_proc_msg0_req(body)
    elif message_type = RA_MSG_TYPE.TYPE_RA_MSG1:
        ret, response = ra_proc_msg1_req(body)
    elif message_type = RA_MSG_TYPE.TYPE_RA_MSG3:
        ret, response = ra_proc_msg3_req(body)
    else:
	print "Received incorrect response"
	return -1
    #TODO: send response back over the socket


def server(bind_address, port):
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serversocket.bind((bind_address, port))
    serversocket.listen(5)
    while True:
        (clientsocket, address) = serversocket.accept()
        handle_connection(clientsocket)
