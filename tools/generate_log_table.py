import numpy as np


if __name__ == "__main__":
    with  open("logtable.c", "wt") as f:
        descstr= """#include "stdint.h"

__attribute__ ((section (".qspi_data")))
const float logtable[]= {

"""
        linvalues = """
__attribute__ ((section (".qspi_data")))
const float linvalues[] = {
"""
        epilogue="""
__attribute__ ((section (".qspi_code")))
float fastlog(float x)
{
    uint32_t c=0;
    if (x < linvalues[0])
    {
        return logtable[0];
    }
    if (x > linvalues[255])
    {
        return logtable[255];
    }
    while(linvalues[c] < x)
    {
        c++;
    }
    return (logtable[c] - logtable[c-1])/(linvalues[c] - linvalues[c-1])*(x-linvalues[c-1]);
}

"""
        f.write(descstr)
        vals = np.arange(0.000001,10.0,(10.0001 - 0.000001)/256.0)
        cnt=0
        for v in vals:

            logv = 20.0*np.log10(v)
            
            f.write("{:.9f}f, ".format(logv))
            #print("0x{:02X}, ".format(logv),end="")
            cnt+= 1
            if cnt%16==0:
                f.write("\r\n")
        f.write("\r\n};\r\n")
        f.write(linvalues)
        cnt=0
        for v in vals:
            f.write("{:.9f}f, ".format(v))
            cnt+= 1
            if cnt%16==0:
                f.write("\r\n")
        f.write("\r\n};\r\n")
        f.write(epilogue)
        f.close()