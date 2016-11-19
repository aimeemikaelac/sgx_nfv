import socket
import struct
import array
# need to install the backported enum for this
from enum import enum

request_struct_size = (8 + 32 + 3*8)/8

IS_SP_REGISTERED = False
GLOBAL_SP_EXTENDED_EPID_GROUP = SP_Extended_EPID_GROUP()
GLOBAL_AUTHENTICATION_TOKEN = 0
GLOBAL_SP_CREDENTIALS = 0

class Service_Provider_ID():
    def __init__():
        #initialize a zeroed 16 byte array
        self.id = array.array('B')
        for i in range(16):
            self.id.append(0)

class SP_Extended_EPID_Group():
    def __init__():
        self.group = -1
        self.spid = Service_Provider_ID()

    def enroll(sp_credentials, p_spid):
        '''
        Enroll with the attestation service. For now, just simulate
        
        sp_credentials (int): service provider credentials
        p_spid (Service_Provider_ID): service provider ID - a 16-byte char array

        Returns:
            return_code (int): 0 if error, 1 otherwise
            authentication_token (int): simulation authentication token - 0 for now, since we don't use it yet
        '''
        #TODO: implement actual enrollment process. For now, use the simulation enrollment
        self.spid = p_spid
        return 0,0



class RA_MSG_TYPE(Enum):
    TYPE_RA_MSG0 = 0
    TYPE_RA_MSG1 = 1
    TYPE_RA_MSG2 = 2
    TYPE_RA_MSG3 = 3
    TYPE_RA_ATT_RESULT = 4

def RA_SAMP_RESPONSE():
    def __init__(message_type, status, body):
        self.message_type = message_type
	self.status= status
	self.body = body

def ra_proc_msg0_req(request_body):
    '''
    Sample processing of SGX msg0. Since enrollment is not implemented, just return 0
    for most things
    '''
    extended_epid_group_id = struct.unpack('I', request_body)
    if not IS_SP_REGISTERED or extended_epid_group is not SP_EXTENDED_EPID_GROUP:
        ret, GLOBAL_AUTHENTICATION_TOKEN = GLOBAL_SP_EXTENDED_EPID_GROUP.enroll(GLOBAL_SP_CREDENTIALS, GLOBAL_AUTHENTICATION_TOKEN)
        if ret is not 0:
            return -1
        else:
            IS_SP_REGISTERED = True
            return 0
    else:
        return 0

def ra_proc_msg1_req(request_body):
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
