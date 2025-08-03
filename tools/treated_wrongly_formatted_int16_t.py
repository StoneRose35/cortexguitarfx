import re
from pathlib import Path
import os.path


def replaceFct(m: re.Match):
    val = int(m.group(),16)
    if (val > 32767):
        val = val - 65536
        return "-0x{:x}".format(-val)
    return "0x{:x}".format(val)


def replaceInString(initial):
    hexpattern = re.compile("0x[0-9a-f]{4}")
    return re.sub(hexpattern,replaceFct,initial)


def replaceInFile(fname):
    rootpath = os.path.abspath(os.path.dirname(__file__))
    path = os.path.join(rootpath, fname)
    with open(path,"rt") as f:
        stringcontent = f.read()
        hexpattern = re.compile("0x[0-9a-f]{4}")
        replaced_code = re.sub(hexpattern,replaceFct,stringcontent)
        with open(path.replace(".c","_corr.c"),"wt" ) as fw:
            fw.write(replaced_code)
        pass

if __name__ == "__main__":
    #print(replaceInString("0x62c, 0x674, 0x7d6, 0xbc4, 0x1312, 0x1ea7, 0x2e33, 0x3b3a, 0x3a9e, 0x29bf, 0x15f5, 0x878, 0xf984, 0xebee, 0xe813, 0xe93e, 0xec34, 0xf3d3, 0xfd12, 0x312, 0x5bf, 0x6eb, 0x5da, 0x487, 0x614, 0x771, 0x837, 0x784, 0x299, 0xfc8e, 0xf9f8, 0xfbd5, 0x2b, 0x44c, 0x599, 0x2f3, 0x43, 0x2, 0xfe5b, 0xfc3c, 0xfd4d, 0xb7, 0x4ac, 0x823, 0xa3b, 0xa6a, 0x915, 0x74c, 0x69c, 0x6e5, 0x73e, 0x6cc, 0x4bf, 0x215, 0xffae, 0xfd9b, 0xfde5, 0xffd2, 0x222, 0x3cd, 0x50d, 0x5fa, 0x659, 0x61f"))
    replaceInFile("../Inc/pipicofx/007_AmpModelHighGain.hpp")
    pass
