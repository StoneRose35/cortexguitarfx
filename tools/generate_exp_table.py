import numpy as np


if __name__ == "__main__":
    with  open("exptable.c", "wt") as f:
        descstr= """#include "stdint.h"


__attribute__ ((section (".qspi_data")))
const float exptable[]= {

"""
        dbvalues = """
__attribute__ ((section (".qspi_data")))
const float dbvalues[] = {
"""
        epilogue = """
__attribute__ ((section (".qspi_code")))
float fastexp(float x)
{
    uint32_t c=0;
    if (x < -120.0f)
    {
        return exptable[0];
    }
    if (x > 20.0f)
    {
        return dbvalues[280];
    }
    while(dbvalues[c] < x)
    {
        c++;
    }
    return (exptable[c] - exptable[c-1])/(dbvalues[c] - dbvalues[c-1])*(x-dbvalues[c-1]);
}

"""
        f.write(descstr)
        vals = np.arange(-120.0,20.5,(20.51+120.0)/256)
        cnt=0
        for v in vals:

            expv = np.power(10.,v/20.0)
            
            f.write("{:.9f}f, ".format(expv))
            cnt+=1
            if cnt%16 == 0:
                f.write("\r\n")

        f.write("\r\n};\r\n")
        f.write(dbvalues)
        cnt=0
        for v in vals:
            f.write("{:.9f}f, ".format(v))
            cnt+=1
            if cnt%16 == 0:
                f.write("\r\n")
        f.write("\r\n};\r\n")
        f.write(epilogue)
        f.close()