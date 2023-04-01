import numpy as np
if __name__ == "__main__":
    # pll range from 150 mhz to 960 mhz
    f_hse=16
    f_audio = 0.048*256
    residual=999999999.0
    for div_m3 in range(1,12):
        for div_n3 in range(4,512):
            f_vco = f_hse/div_m3*div_n3
            if 960 >=f_vco >= 150:
                for div_p3 in range(1,129):
                    f_pll = f_hse/div_m3*div_n3/div_p3
                    if f_pll >= f_audio:
                        mckdiv = np.floor(f_pll/f_audio/2)*2
                        if mckdiv < 64:
                            res = np.abs(f_pll - mckdiv*f_audio)
                            if res < residual:
                                residual = res
                                print("residual: {:.6f}MHz, m3: {}, n3: {}, f_vco: {:.0f}, f_pll: {:.0f}, p3: {}, mckdiv: {}"
                                    .format(res,div_m3,div_n3,f_vco,f_pll,div_p3,mckdiv))
    
