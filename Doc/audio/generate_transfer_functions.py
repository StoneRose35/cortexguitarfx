from numpy import ushort
import distortion_modeling
import numpy as np
import matplotlib.pyplot as plt

def generate_generic_curve(xa, xb, ya, yb,doplot=True):
    t1 = distortion_modeling.TransferFunction(xa, xb, ya, yb)
    xvals = np.linspace(-1.,1.*63./64.,128)
    yvals = []
    bytearray=""
    floatarray=""
    for e in xvals:
        yvals.append(t1.compute(e))
        bytearray += "0x{:x}, ".format(ushort(yvals[-1]*32767))
        floatarray += "{:f}f, ".format(yvals[-1])
    print(bytearray)
    print("\r\n")
    print(floatarray)
    if doplot is True:
        plt.plot(xvals, yvals)
        plt.show()

def generate_soft_overedrive():
    generate_generic_curve(0.5, 0.97, 0.75, 0.999)

def generate_default_distortion():
    generate_generic_curve(0.5, 0.6, 0.85, 0.97)

def generate_distortion():
    generate_generic_curve(0.2, 0.36, 0.80, 0.99)

def generate_curved_overdrive():
    generate_generic_curve(0.2, 0.97, 0.5, 0.99)

def generate_asymmetric_od():

    t1 = distortion_modeling.GainReductedTransferFunction(0.27,0.35,0.3,0.125/2,-0.7,-0.99,-0.8,0.0)
    xvals = np.linspace(-1.,1.*63./64.,128)
    yvals = []
    bytearray=""
    floatarray=""
    for e in xvals:
        yvals.append(t1.compute(e))
        bytearray += "0x{:x}, ".format(ushort(yvals[-1]*32767))
        floatarray += "{:f}f, ".format(yvals[-1])
    print(bytearray)
    print("\r\n")
    print(floatarray)
    plt.plot(xvals, yvals,".-k")
    plt.show()


def generate_unity():

    xvals = np.linspace(-1.,1*63./64.,128)
    yvals = []
    bytearray=""
    floatarray=""
    for e in xvals:
        yvals.append(e)
        bytearray += "0x{:x}, ".format(ushort(yvals[-1]*32767))
        floatarray += "{:f}f, ".format(yvals[-1])
    print(bytearray)
    print("\r\n")
    print(floatarray)
    plt.plot(xvals, yvals)
    plt.show()

if __name__ == "__main__":
    #generate_soft_overedrive()
    #generate_default_distortion()
    #generate_unity()
    #generate_distortion()
    #generate_curved_overdrive()
    generate_asymmetric_od()
