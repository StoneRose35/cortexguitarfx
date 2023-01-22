import numpy as np


if __name__ == "__main__":
    with  open("logtable.c", "wt") as f:
        descstr= """#include "stdint.h"
// logarithm table covering the range of 1/32768 up to 32767/32768 typicall expressed as fixed point 2byte (q15) integers as 32768 + 7256.64*log10(x) 
// thus mapping to the full q15 range


const int16_t logtable[]= {

"""
        epilogue="""
int16_t fastlog(int16_t idx)
{
    return *(logtable + idx);
}

"""
        qfact =  np.floor(-32767/np.log10(1./32768.)*1000000.)/1000000.
        f.write(descstr)
        vals = range(0,32768,1)
        for v in vals:
            if v == 0:
                logv = 0
            else:
                logv = int(32768 + qfact*np.log10(v/32768))
            
            f.write("0x{:02X}, ".format(logv))
            #print("0x{:02X}, ".format(logv),end="")
            if v%16== 0:
                f.write("\r\n")
                #print("")
            if v%1024== 0:
                f.write("\r\n")
                #print("")
        f.write("\r\n};\r\n")
        f.write(epilogue)
        f.close()