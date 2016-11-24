#!/usr/bin/python
from sample_crypto_wrappermodule import *

if __name__ == "__main__":
    ga_x, ga_y, ga_priv_r = generateDHKeys()
    print "Ga x:      0x{}".format(ga_x.encode('hex'))
    print "Ga y:      0x{}".format(ga_y.encode('hex'))
    print "Ga priv r: 0x{}".format(ga_priv_r.encode('hex'))