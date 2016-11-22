import array
from enum import Enum


class Service_Provider_ID():
    def __init__(self):
        #initialize a zeroed 16 byte array
        self.id = array.array('B')
        for i in range(16):
            self.id.append(0)

class SP_Extended_EPID_Group():
    def __init__(self):
        self.group = -1
        self.spid = Service_Provider_ID()

    def enroll(self, sp_credentials, p_spid):
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
    
    def get_sigrl(self, gid):
        '''
        Get the Signature Revocation List (sigrl) from Intel. For now, only simulate
        
        gid (4 byte array): Group ID for the EPID key that we are getting the revocation list for
        
        returns:
            ret(int): return code indicating success/failure
            sigrl(list): a list containing te revocation list (empty list for now)
        '''
        return 0, []
    
class RA_SAMP_RESPONSE():
    def __init__(self, message_type, status, body):
        self.message_type = message_type
        self.status = status
        self.body = body
        
class ECCDH_Public_Key():
    def __init__(self, x, y):
        self.x = x
        self.y = y
    
class RA_MSG_TYPE(Enum):
    TYPE_RA_MSG0 = 0
    TYPE_RA_MSG1 = 1
    TYPE_RA_MSG2 = 2
    TYPE_RA_MSG3 = 3
    TYPE_RA_ATT_RESULT = 4