#!/usr/bin/python3
import math
import matplotlib.image as mpimg
import matplotlib.pyplot as plt
import numpy as np
import os.path
import os
import argparse

c_template = """
#ifndef _{4}_H_
#define _{4}_H_
#include "imgDisplay.h"

static const uint8_t {0}_clrdata[]= {{
{1}
}};

static const struct ST7735ImageStruct {0}_streamimg = {{
    .colorbytes = {0}_clrdata,
    .rows = {2},
    .columns = {3}
}};
#endif
"""

c_template_bw = """
#ifndef _{4}_H_
#define _{4}_H_
#include "graphics/bwgraphics.h"

static uint8_t {0}_bwdata[]= {{
{1}
}};

static const struct BwImageStruct {0}_streamimg = {{
    .data = {0}_bwdata,
    .sx = {2},
    .sy = {3}
}};
#endif
"""

c_template_font = """
static const uint8_t {}[{}][{}]={{
{}
}};
"""
template_raw_struct = """
{{
    .colorbytes = {},
    .rows = {},
    .columns = {}
}}
"""

def st7735_encode_color(r,g,b):
    msb = (r & 0xF8) | ((g >> 5) & 0x7)
    lsb = ((g & 0x7) << 5) | ((b & 0xF8) >> 3)
    #print("encoded color, msb: {}, lsb: {}".format(hex(msb),hex(lsb)))
    return msb, lsb

def el_to_byte(el):
    rval = int(el[0] * 255)
    gval = int(el[1] * 255)
    bval = int(el[2] * 255)
    msb, lsb = st7735_encode_color(rval, gval, bval)
    return hex(msb) + ", " + hex(lsb)

def el_to_bw_pixels(el):
    if el[0] > 0 or el[1] > 0 or el[2] > 0:
        return 1
    else:
        return 0
    

def imageToCStream(fname="Rheinisch-Kaltblut-Gespann.png",outfolder=""):
    img = mpimg.imread(fname)
    imgname = fname.split(os.path.sep)[-1].split(".")[0]
    if (len(outfolder) > 0):
        fp = open(os.path.join(outfolder, imgname + ".h"), "wt")
    else:
        fp = open(imgname + ".h", "wt")
    bytearray = ""
    c = 0
    for row in range(img.shape[0]):
        colbytes = map(el_to_byte, img[row])
        colbytes = ", ".join(colbytes)
        colbytes += ",\r\n"
        bytearray += colbytes

    fcontent = c_template.format(imgname, bytearray, int(img.shape[0]), int(img.shape[1]),imgname.upper())
    fp.write(fcontent)
    fp.close()
    
def imageToBWCStream(fname="Rheinisch-Kaltblut-Gespann.png",outfolder=""):
    img = mpimg.imread(fname)
    imgname = fname.split(os.path.sep)[-1].split(".")[0]
    if (len(outfolder) > 0):
        fp = open(os.path.join(outfolder, imgname + ".h"), "wt")
    else:
        fp = open(imgname + ".h", "wt")
    bytearray = ""
    c = 0
    rownr_old=0
    nrows = int(img.shape[0]/8)
    pixeldata = np.zeros(int(img.shape[0]/8)*img.shape[1],dtype="uint8")
    for row in range(img.shape[0]):
        rownr = int(np.floor(c/8))
        bwbits = list(map(el_to_bw_pixels, img[row]))

        bitpos = c - rownr*8
        for p in range(img.shape[1]):
            idx = p*nrows + rownr
            pixeldata[idx] |= (bwbits[p] << bitpos)
        c += 1
    for el in pixeldata:
        bytearray += hex(el) + ", "
    fcontent = c_template_bw.format(imgname, bytearray, int(img.shape[1]), int(img.shape[0]),imgname.upper())
    fp.write(fcontent)
    fp.close()


def fontImageToArray(fname="sm_ascii_16x16.png", sizex=16, sizey=16, offsetx=0, offsety=0, outfolder=""):
    img = mpimg.imread(fname)
    imgname = fname.split(os.path.sep)[-1].split(".")[0]
    if (len(outfolder) > 0):
        fp = open(os.path.join(outfolder, imgname + ".h"), "wt")
    else:
        fp = open(imgname + ".h", "wt")
    asciientries = []
    nfonts = 0
    for col in range(int(img.shape[0]/sizex)):
        for row in range(int(img.shape[1]/sizey)):
            partimg = list(map(lambda x: x[row*sizey+offsety:(row+1)*sizey+offsety], img[col*sizex+offsetx:(col+1)*sizex+offsetx]))
            colarr = np.reshape(partimg, (sizex*sizey, 4))
            bytearray = map(el_to_byte, colarr)
            bytearray = ", ".join(bytearray)
            asciientries.append("{" + bytearray + "}")
            nfonts += 1
    asciientries = "\r\n, ".join(asciientries)
    fonttable = c_template_font.format(imgname,nfonts,sizex*sizey*2, asciientries)
    fp.write(fonttable)
    fp.close()





def oscillator_freq_calc():
        # computed the possible division factors for the pll
    f_xosc = 12000000.

    f_ws2818 = 800000.

    mult_pio = 10
    pio0_fact = 1

    feedbck = 1
    postdiv1 = 1
    postdiv2 = 1

    f_sys = f_xosc
    params_array = []
    bestparams = None
    while f_sys < 133000000:
        f_sys = f_ws2818 * mult_pio * pio0_fact
        if f_sys > f_xosc:
            postdiv1 = 1
            postdiv2 = 1
            while postdiv1 < 8:
                postdiv2 = 1
                while postdiv2 < 8:
                    if f_sys*postdiv1*postdiv2 > 400000000 and f_sys*postdiv1*postdiv2 < 1600000000:
                        f_vco = f_sys*postdiv1*postdiv2
                        feedbk_f = f_vco / f_xosc
                        feedbk = math.floor(feedbk_f)
                        f_vco_appr = feedbk * f_xosc
                        f_sys_approx = f_vco_appr/postdiv1/postdiv2
                        f_ws2812_approx = f_sys_approx/mult_pio/pio0_fact
                        f_ws2812_err = math.fabs(f_sys - f_ws2812_approx)
                        params = {"feedbk": feedbk, "pio_clock": pio0_fact, "postdiv1": postdiv1,
                                  "postdiv2": postdiv2, "f_vco": f_vco_appr,"f_sys": f_vco_appr/postdiv1/postdiv2,
                                  "f_ws2812": f_ws2812_approx,
                                  "f_ws_2812_err": f_ws2812_err}
                        print("Feedback Multiplier: {}".format(params["feedbk"]))
                        print("PIO0 Clock Divider: {}".format(params["pio_clock"]))
                        print("Post Divider 1: {}".format(params["postdiv1"]))
                        print("Post Divider 2: {}".format(params["postdiv2"]))
                        print("VCO Frequency: {}".format(params["f_vco"]))
                        print("System Frequency: {}".format(params["f_sys"]))
                        print("ws2812 Frequency: {}".format(params["f_ws2812"]))
                        print("")
                        if bestparams is None:
                            bestparams = params
                        elif bestparams["f_ws_2812_err"] > params["f_ws_2812_err"]:
                            bestparams = params
                        params_array.append(params)
                    postdiv2 += 1
                postdiv1 += 1
        pio0_fact += 1
    print("\n\n\nBest Parameter Set")
    print("Feedback Multiplier: {}".format(bestparams["feedbk"]))
    print("PIO0 Clock Divider: {}".format(bestparams["pio_clock"]))
    print("Post Divider 1: {}".format(bestparams["postdiv1"]))
    print("Post Divider 2: {}".format(bestparams["postdiv2"]))
    print("VCO Frequency: {}".format(bestparams["f_vco"]))
    print("System Frequency: {}".format(bestparams["f_sys"]))
    print("ws2812 Frequency: {}".format(bestparams["f_ws2812"]))




if __name__ == "__main__":
    asset_path = "../Assets"
    font_path = "../Assets/fonts"
    image_inc_path = "../Inc/images"
    font_inc_path = "../Inc/fonts"
    parser = argparse.ArgumentParser()
    parser.add_argument("-calcSysFreqs",help="calculate oscillator frequencies",action="store_true")
    parser.add_argument("-generateAssets",help="generate images and font asset headers",action="store_true")
    parser.add_argument("-convertImg",help="convert specific image to c header as 16bit color image (for ST7735)")
    parser.add_argument("-convertBwImg",help="convert specific image to c header as black/white image (for SSD1306)")

    args = parser.parse_args()
    if args.calcSysFreqs is False and args.generateAssets is False and args.convertImg is None and args.convertBwImg is None:
        parser.print_help()
    else:
        if args.calcSysFreqs is True:
            oscillator_freq_calc()
        if args.generateAssets is True:
            dircontent = os.listdir(asset_path)
            for el in dircontent:
                full_path = os.path.join(asset_path,el)
                if os.path.isfile(full_path) and full_path.lower().endswith("png"):
                    imageToCStream(full_path, image_inc_path)
            fontImageToArray("../Assets/fonts/Codepage737.png", 16, 9, 4, 4,font_inc_path)
            fontImageToArray("../Assets/fonts/sm_ascii_16x16.png",16,16,0,0,font_inc_path)
        elif args.convertImg is not None:
            # dircontent = os.listdir(asset_path)
            full_path = args.convertImg # os.path.join(asset_path,args.convertImg)
            if os.path.isfile(full_path) and full_path.lower().endswith("png"):
                imageToCStream(full_path, "../Inc/images")
        elif args.convertBwImg is not None:
            full_path = args.convertBwImg # os.path.join(asset_path,args.convertImg)
            if os.path.isfile(full_path) and full_path.lower().endswith("png"):
                imageToBWCStream(full_path, "../Inc/images")



    """
    imageToCStream("../Assets/OK_32x32.png", "../Inc/images")
    imageToCStream("../Assets/back_32x32.png", "../Inc/images")


    imageToCStream("../Assets/kaltblut3.png", "../Inc/images")
    imageToCStream("../Assets/kaltblut2.png", "../Inc/images")
    imageToCStream("../Assets/kaltblut1.png", "../Inc/images")
    imageToCStream("../Assets/clock_32x32.png", "../Inc/images")
    imageToCStream("../Assets/drafthorse_32x32.png", "../Inc/images")
    imageToCStream("../Assets/bulb_off_24x24.png", "../Inc/images")
    imageToCStream("../Assets/bulb_on_24x24.png", "../Inc/images")
    """
    #st7735_encode_color(254, 255, 179)
