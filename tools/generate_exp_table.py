import numpy as np


if __name__ == "__main__":
    with  open("exptable.c", "wt") as f:
        descstr= """#include "stdint.h"
// exponential function table covering the range of 1/32768 up to 32767/32768 typically expressed as fixed point 2byte (q15) integers as 10^((x-32768)/7256.64) 
// thus mapping to the full q15 range


const int16_t exptable[]= {

"""
        qfact =  np.floor(-32767/np.log10(1./32768.)*1000000.)/1000000.
        f.write(descstr)
        vals = range(0,32768,1)
        for v in vals:
            if v == 0:
                expv = 0
            else:
                expv = int(np.power(10.,(v-32768.)/qfact)*32768)
            
            f.write("0x{:02X}, ".format(expv))
            #print("0x{:02X}, ".format(logv),end="")
            if v%16== 0:
                f.write("\r\n")
                #print("")
            if v%1024== 0:
                f.write("\r\n")
                #print("")
        f.write("\r\n};\r\n")
        f.close()