import numpy as np
import scipy.signal
import scipy.signal as signal
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
import math
import cmath


def int32(x):
    if x > (1 << 31) or x < -(1 << 31):
        raise OverflowError
    xint=x & 0xFFFFFFFF
    if xint > (1 << 31):
        xint = xint - (1 << 32)
    return xint


class AudioFilter:
    def __init__(self,a,b,do_overflow=True,bit_res=16):
        self.a=a
        self.b=b
        self.w=[int(0),int(0),int(0)]
        self.do_overflow=do_overflow
        self.bit_res = bit_res

    def apply(self, sample):
        out = 0
        if self.do_overflow is True:
            self.w[0] = int32(int(sample) - (int32(self.a[0]*self.w[1]) >> (self.bit_res-1)) - (int32(self.a[1]*self.w[2]) >> (self.bit_res-1)))
            out = int32(int32((int32(self.b[0]*self.w[0])>> (self.bit_res-1))) + int32((int32(self.b[1]*self.w[1]) >> (self.bit_res-1))) + int32((int32(self.b[2]*self.w[2]) >> (self.bit_res-1)) ))
        else:
            self.w[0] = (int(sample) - ((self.a[0]*self.w[1]) >> (self.bit_res-1)) - ((self.a[1]*self.w[2]) >> (self.bit_res-1)))
            out = ((((self.b[0]*self.w[0])>> (self.bit_res-1))) + (((self.b[1]*self.w[1]) >> (self.bit_res-1))) + (((self.b[2]*self.w[2]) >> (self.bit_res-1)) ))
        self.w[2]=self.w[1]
        self.w[1]=self.w[0]
        return out

def compute_td_energy_fraction(bd_orig,ad_orig,do_overflow,sample_size=16):
    bd = bd_orig
    ad = ad_orig
    bvals = np.array(bd * ((1 << (16 - 1)) - 1)).astype(int)
    avals = np.array(ad[1:] * ((1 << (16 - 1)) - 1)).astype(int)
    testFilter = AudioFilter(avals, bvals,do_overflow=do_overflow,bit_res=16)
    tdOutput = []
    avgEnergy = 0
    for c in range(1024):
        inputVal = int((np.random.random() - 0.5) * (32767 >> (16 - sample_size)) * 2)
        avgEnergy += inputVal*inputVal
        #if c == 0:
        #    inputVal = 32767 >> (16-sample_size)
        #else:
        #    inputVal = 0
        try:
            tdOutput.append(testFilter.apply(inputVal))
        except OverflowError:
            return 42.0
    avgEnergy = np.sqrt(avgEnergy)
    energy = np.sqrt(np.sum(np.square(tdOutput)))
    return energy/avgEnergy


def get_phase_increments(f_sampling=48000,oversampling=2,bitres=32):
    notes = range(128)
    fs=f_sampling*oversampling
    phaseincs=[]
    f_synths = []
    for note in notes:
        notefreq = math.pow(2.,(note-64)/12.)*440.
        phaseinc = notefreq/fs*((1 << bitres))
        f_synth = int(phaseinc)*fs/((1 << bitres))
        f_synths.append(f_synth)
        if (phaseinc > (1 << (bitres-1))):
            print("note {} unavailable".format(note))
        else:
            phaseincs.append(int(phaseinc))
            #print("{},{}, {}, {}".format(note, notefreq, int(phaseinc),f_synth))
            #print("{},".format(int(phaseinc)))
    return phaseincs, f_synths

def get_sine_table():
    bitres = 16
    nvals = 256
    sinetable=[]
    for c in range(nvals):
        print("{},".format(int(math.cos(c/nvals*math.pi*2)*((1 << (bitres-1)) - 1))))
        sinetable.append(int(math.cos(c/nvals*math.pi*2)*((1 << (bitres-1)) - 1)))
    return sinetable

def design_iir_filter(rs=20,fc=None,fs=48000,do_overflow=True,sample_size=16,scaling=None,type="cheby2",ftype="lowpass"):
    if fc is None:
        fc = fs/2
    if ftype == "highpass" or ftype=="lowpass":
        order = 2
    else:
        order = 1
    auto_sample_size = sample_size is None
    if type == "cheby2":
        sos = signal.cheby2(order,rs, fc, analog=False, btype=ftype, output='sos', fs=fs)
        ba = signal.butter(order, fc, analog=False, btype=ftype, output='ba', fs=fs)
    elif type == "cheby1":
        sos = signal.cheby1(order, rs, fc, analog=False, btype=ftype, output='sos', fs=fs)
        ba = signal.butter(order, fc, analog=False, btype=ftype, output='ba', fs=fs)
    elif type == "butter":
        sos = signal.butter(order, fc, analog=False, btype=ftype, output='sos', fs=fs)
        ba = signal.butter(order, fc, analog=False, btype=ftype, output='ba', fs=fs)
    else:
        sos= [[1.,0.,0.], [1.,0.,0.]]
    return sos

def plot_iir_filter(sos,do_plot=True,fs=48000,do_overflow=True,sample_size=16,scaling=None,fc=None,ftype=None,type=None,rs=0):

    auto_sample_size = sample_size is None
    cnt=0
    sos_returned = []
    for second_order_filter in sos:
        bd = second_order_filter[:3]
        ad = second_order_filter[3:]

        if auto_sample_size is True:
            sample_size_dyn = 16
            max_val = ((1 << (16 - 1)) - 1)
            bvals = np.array(bd * max_val).astype(int)
            avals = np.array(ad[1:] * max_val).astype(int)
            done = False
            while done is False:
                energy_fraction = compute_td_energy_fraction(bd,ad,do_overflow=do_overflow,sample_size=sample_size_dyn)
                done = bool(energy_fraction < 2.01)
                if done is False:
                    sample_size_dyn -= 1
                print("sample size: {}, energy fraction: {}".format(sample_size_dyn,energy_fraction))
            sample_size = sample_size_dyn

        max_val = ((1 << (16 - 1)) - 1)

        bvals = np.array(bd * max_val).astype(int)
        avals = np.array(ad[1:] * max_val).astype(int)
        # pole calculation
        p1 = (-float(avals[0]) + cmath.sqrt(float(avals[0])*float(avals[0]) - 4*float(max_val)*float(avals[1])))/(2.*float(max_val))
        p2 = (-float(avals[0]) - cmath.sqrt(float(avals[0]) * float(avals[0]) - 4 * float(max_val) * float(avals[1]))) /(2.*float(max_val))
        z1 = (-float(bvals[1]) + cmath.sqrt(float(bvals[1])*float(bvals[1]) - 4*float(bvals[0])*float(bvals[2])))/(2.*float(max_val))
        z2 = (-float(bvals[1]) - cmath.sqrt(float(bvals[1]) * float(bvals[1]) - 4 * float(bvals[0]) * float(bvals[2]))) /(2.*float(max_val))
        print("pole 1 at {:.3f}/{:.3f}, absolute value: {:.3f}".format(np.real(p1),np.imag(p1),abs(p1)))
        print("pole 2 at {:.3f}/{:.3f}, absolute value: {:.3f}".format(np.real(p2), np.imag(p2), abs(p2)))
        print("zero 1 at {:.3f}/{:.3f}".format(np.real(z1),np.imag(z1)))
        print("zero 2 at {:.3f}/{:.3f}".format(np.real(z2), np.imag(z2)))
        z,p,k = scipy.signal.tf2zpk(bd,ad)
        print("gain: {:.3f}".format(k))
        if do_plot is True:
            print("B coefficients: {}".format(bvals))
            print("A coefficients: {}".format(avals))
            print("sample size: {}".format(sample_size))

            wz, hz = signal.freqz(bd, ad)
            fig = plt.figure(figsize=[7.2,7.2],constrained_layout=True)
            gs = GridSpec(3,2,figure=fig)

            axs0 = fig.add_subplot(gs[0,0])
            freqs = wz*fs/2./np.pi
            axs0.plot(freqs,20.*np.log10(abs(hz)),linewidth=2,color="blue")
            axs0.grid(color="gray",linestyle="--",which="major",axis="both")
            axs0.grid(color="gray", linestyle="--", which="minor", axis="x")
            axs0.axis([20,fs/2,-60,10])
            axs0.set_xscale("log")
            axs0.set_title("Frequency Response")
            axs0.set_ylabel("Gain [dB]")
            axs0.set_xlabel("Frequency [Hz]")

            axs1 = fig.add_subplot(gs[1, 0])
            axs1.plot(freqs,np.unwrap(np.arctan2(np.real(hz),np.imag(hz)))*180./np.pi,linewidth=2,color="blue")
            axs1.grid(color="gray",linestyle="--",which="major",axis="both")
            axs1.grid(color="gray", linestyle="--", which="minor", axis="x")
            axs1.set_xlim([20,fs/2])
            axs1.set_xscale("log")
            axs1.set_title("Phase response")
            axs1.set_ylabel("Phase [deg]")
            axs1.set_xlabel("Frequency [Hz]")

            # time-domain test
            x_time_domain = np.linspace(0,1./fs*1000,1024)
            testFilter = AudioFilter(avals,bvals,do_overflow=do_overflow,bit_res=16)
            tdOutput=[]
            for c in range(1024):
                if c==0:
                    inputVal=32767 >> (16-sample_size)
                else:
                    inputVal = 0
                tdOutput.append(testFilter.apply(inputVal))
            axs2=fig.add_subplot(gs[2,0])
            axs2.plot(x_time_domain,tdOutput, ".-r", label="positive pulse")
            testFilter = AudioFilter(avals,bvals,do_overflow=do_overflow,bit_res=16)
            tdOutput=[]
            for c in range(1024):
                if c==0:
                    inputVal=-32767 >> (16-sample_size)
                else:
                    inputVal = 0
                tdOutput.append(testFilter.apply(inputVal))
            axs2.plot(x_time_domain,tdOutput, ".-g", label="negative pulse")
            axs2.set_ylim([-(32768 >> (16-sample_size))*1.007, (32768 >> (16-sample_size))*1.007])
            axs2.set_yticks(np.arange(start=-(32768 >> (16-sample_size)),stop=(32768 >> (16-sample_size)),step=(32768 >> (16-sample_size))/4))
            axs2.set_title("Time Domain response")
            axs2.set_xlabel("Time [ms]")
            axs2.set_ylabel("Ampl. [Int16]")
            axs2.grid(color="gray", linestyle="--", which="both", axis="y")
            axs2.legend(loc="upper right")

            testFilter = AudioFilter(avals, bvals, do_overflow=do_overflow, bit_res=16)
            tdOutput=[]
            for c in range(1024):
                inputVal = int((np.random.random()-0.5)*(32767 >> (16-sample_size))*2)
                tdOutput.append(testFilter.apply(inputVal))
            axs4 = fig.add_subplot(gs[2,1])
            axs4.plot(x_time_domain,tdOutput, ".-g", label="negative pulse")
            axs4.set_ylim([-(32768 >> (16-sample_size))*1.007, (32768 >> (16-sample_size))*1.007])
            axs4.set_yticks(np.arange(start=-(32768 >> (16-sample_size)),stop=(32768 >> (16-sample_size)),step=(32768 >> (16-sample_size))/4))
            axs4.set_title("Noise response")
            axs4.set_xlabel("Time [ms]")
            axs4.set_ylabel("Ampl. [Int16]")
            axs4.grid(color="gray", linestyle="--", which="both", axis="y")

            axs3 = fig.add_subplot(gs[0,1])
            if type == "cheby1":
                filter_type_displ = "Chebychev Type I, "
            elif type == "cheby2":
                filter_type_displ = "Chebychev Type II, "
            elif type == "butter":
                filter_type_displ = "Butterworth, "
            else:
                filter_type_displ = "Custom, "
                rs= 0.0
            filter_type_displ += ftype
            cnt+=1
            fsize=1.
            font_size=8.
            axs3.text(0.02,16-1*fsize,"Second-Order Section {}/{}".format(cnt,len(sos)),fontsize=font_size)
            axs3.text(0.02,16-3*fsize,"Filter Type: \n    " + filter_type_displ,fontsize=font_size)
            freq_atten_displ = "Cutoff / Attenuation: \n    {}Hz".format(fc)
            if type!="butter":
                freq_atten_displ += ", Attenuation: {}dB".format(rs)
            axs3.text(0.02, 16 - 5 * fsize, freq_atten_displ,fontsize=font_size)
            axs3.text(0.02,16-7*fsize,"B coefficients: \n    {}".format(bvals),fontsize=font_size)
            axs3.text(0.02,16-9*fsize,"A coefficients: \n    {}".format(avals),fontsize=font_size)
            axs3.text(0.02, 16-11*fsize, "bit resolution: \n    {}".format(sample_size),fontsize=font_size)
            axs3.text(0.02, 16-13*fsize, "gain: \n    {:.6f}".format(k),fontsize=font_size)
            axs3.tick_params(bottom=False, labelbottom=False, labelleft=False, left=False)
            axs3.set_ylim([0,16])


            axs6=fig.add_subplot(gs[1,1])
            circle_angles=np.linspace(0,2.*np.pi,512)
            circle_x=np.sin(circle_angles)
            circle_y=np.cos(circle_angles)
            axs6.plot(circle_x,circle_y,"-b")
            axs6.plot(np.real([z1,z2]),np.imag([z1,z2]),"ob",fillstyle='none',label="Zeros")
            axs6.plot(np.real([p1,p2]),np.imag([p1,p2]),"+b",label="Poles")
            axs6.set_title("Poles / Zeros")
            axs6.legend(loc="upper right")
            axs6.set_xticks([-1., -0.5, 0., 0.5, 1])
            axs6.set_yticks([-1., -0.5, 0., 0.5, 1])
            axs6.grid(color="gray", linestyle="--", which="both")
            axs6.axis('equal')


            plt.show()

        sos_returned.append({"b": bvals, "a": avals, "sample_bit_size": sample_size})
    return sos_returned

def design_and_plot_iir_filter(do_plot=True,rs=20,fc=None,fs=48000,do_overflow=True,sample_size=16,scaling=None,type="cheby2",ftype="lowpass"):
    sos = design_iir_filter(rs=rs,fc=fc,fs=fs,sample_size=sample_size,scaling=scaling,type=type,ftype=ftype)
    plot_iir_filter(sos,do_plot=do_plot,fs=fs,do_overflow=do_overflow,sample_size=sample_size,scaling=scaling,fc=fc, ftype=ftype,type=type,rs=rs)

def plot_sos_frequency_response(sosections,fs=48000,nfreqs=512):

    h_tot = np.ones(nfreqs)
    w = np.linspace(0, fs / 2., nfreqs)
    for sos in sosections:
        b = sos[0]
        a = sos[1]
        w, h = scipy.signal.freqz(b, a, worN=nfreqs, fs=fs)
        h_tot = h_tot * h

    fig, axxes = plt.subplots(2, 1)
    axxes[0].plot(w, 20. * np.log10(abs(h_tot)) - max(20.* np.log10(abs(h_tot))), ".-r")
    axxes[0].set_xscale("log")
    axxes[0].set_ylim([-60,0])

    axxes[1].plot(w, np.unwrap(np.arctan2(np.real(h_tot), np.imag(h_tot))), ".-r")
    axxes[1].set_xscale("log")
    plt.show()

def design_and_plot_oversampling_lowpass_cheby(do_plot=False,to_integer=True,rs=20,oversampling=2,fc=None):
    ftype = 'lowpass'
    fs = 48000
    if fc is None:
        fc = fs/2
    order = 2
    sample_size = 16
    f0 = fc/(fs*oversampling)*2.
    bd, ad = signal.cheby2(order,rs, f0, analog=False, btype=ftype, output='ba')

    bvals = np.array(bd * ((1 << (sample_size-1)) - 1)).astype(int)
    avals = np.array(ad*((1 << (sample_size-1)) -1)).astype(int)
    if do_plot is True:
        print("B coefficients: {}".format(bvals))
        print("A coefficients: {}".format(avals))

        wz, hz = signal.freqz(bd, ad)

        freqs = wz*(fs*oversampling)/2./np.pi
        plt.subplot(2,1,1)
        plt.plot(freqs,20.*np.log10(abs(hz)))
        #plt.axis([20,(fs*oversampling)/2,-60,0])
        #plt.xscale("log")
        plt.subplot(2,1,2)
        plt.plot(freqs,np.unwrap(np.arctan2(np.real(hz),np.imag(hz)))*180./np.pi)
        #plt.xscale("log")
        plt.show()
    if to_integer is True:
        return avals,bvals
    else:
        return ad,bd

def design_and_plot_oversampling_lowpass_butter(do_plot=False,to_integer=True,oversampling=2,fc=None):
    ftype = 'lowpass'
    fs = 48000
    if fc is None:
        fc = fs/2
    order = 2
    sample_size = 16

    f0 = fc/(fs*oversampling)*2.
    bd, ad = signal.butter(order, f0, analog=False, btype=ftype, output='ba')

    bvals = np.array(bd * ((1 << (sample_size-1)) - 1)).astype(int)
    avals = np.array(ad*((1 << (sample_size-1)) -1)).astype(int)
    if do_plot is True:
        print("B coefficients: {}".format(bvals))
        print("A coefficients: {}".format(avals))

        wz, hz = signal.freqz(bd, ad)

        freqs = wz*(fs*oversampling)/2./np.pi
        plt.subplot(2,1,1)
        plt.plot(freqs,20.*np.log10(abs(hz)))
        plt.axis([20,(fs*oversampling)/2,-60,0])
        #plt.xscale("log")
        plt.subplot(2,1,2)
        plt.plot(freqs,np.unwrap(np.arctan2(np.real(hz),np.imag(hz)))*180./np.pi)
        #plt.xscale("log")
        plt.show()
    if to_integer is True:
        return avals,bvals
    else:
        return ad, bd

def design_and_plot_oversampling_lowpass_fir(do_plot=False, to_integer=True, oversampling=2,fc=None):
    firsize = 4
    firwidth = 0.1
    fs=48000
    sample_size = 16
    bd = signal.firwin(firsize, fc / oversampling, firwidth)
    ad = np.zeros(firsize)
    bvals = np.array(bd*((1 << (sample_size))-1)).astype(int)
    avals = np.zeros(firsize)

    if do_plot is True:
        print("B coefficients: {}".format(bvals))
        print("A coefficients: {}".format(avals))

        wz, hz = signal.freqz(bd)

        freqs = wz*(fs*oversampling)/2./np.pi
        plt.subplot(2,1,1)
        plt.plot(freqs,20.*np.log10(abs(hz)))
        plt.axis([20,(fs*oversampling)/2,-60,0])
        #plt.xscale("log")
        plt.subplot(2,1,2)
        plt.plot(freqs,np.unwrap(np.arctan2(np.real(hz),np.imag(hz)))*180./np.pi)
        #plt.xscale("log")
        plt.show()
    if to_integer is True:
        return avals,bvals
    else:
        return ad,bd

def get_square(current_phase,sample_size=16):
    if ((current_phase & 0x80000000) != 0):
        waveformval = (1 << (sample_size - 1))
    else:
        waveformval = -(1 << (sample_size - 1))
    return waveformval

def get_saw(current_phase):
    waveformval = ((current_phase >> 16) & 0xFFFF) - 0x7FFF
    return waveformval


def design_oversampling_comb_lp_filter(rs=3,oversampling=4,fs=48000):
    for c in range(oversampling-1):
        f_cutoff = (np.tan((fs/2*(c+1))/((oversampling*fs)/2.))*oversampling*fs/2.)/np.sqrt(2)
        print("Chebychev filter @{}Hz, attenuation:{}dB".format(int(f_cutoff),rs))
        design_and_plot_oversampling_lowpass_cheby(True, oversampling=oversampling, rs=rs, fc=f_cutoff)
    print("Butterworth filter @12000Hz")
    design_and_plot_oversampling_lowpass_butter(True, oversampling=oversampling, fc=12000)


if __name__ == "__main__":
    notenr = 128
    sample_rate=48000
    oversampling = 4
    phaseincr_bitsize=36
    rs=3
    #design_and_plot_iir_filter(True,rs=10,fc=170,type="cheby2",ftype="highpass")

    ord = scipy.signal.cheb1ord(200./24000.,20./24000,1,50,analog=False)
    # manual modeling attempt of a guitar speaker using 4 iir filters
    iirfilters=[]
    sos = design_iir_filter(6,3500,type="cheby1",ftype="lowpass")
    design_and_plot_iir_filter(True,rs=6,fc=3500,do_overflow=True,sample_size=None,type="cheby1",ftype="lowpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])
    sos = design_iir_filter(6,2500,type="cheby1",ftype="lowpass")
    design_and_plot_iir_filter(True, rs=6, fc=2500, do_overflow=True, sample_size=None, type="cheby1", ftype="lowpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])
    sos=design_iir_filter(16, 120, type="cheby1", ftype="highpass")
    design_and_plot_iir_filter(True, rs=16, fc=12000, do_overflow=True, sample_size=None, type="cheby1", ftype="highpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])
    sos=design_iir_filter(1, 300, type="butter", ftype="highpass")
    design_and_plot_iir_filter(True, rs=1, fc=300, do_overflow=True, sample_size=None, type="butter", ftype="highpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])

    plot_sos_frequency_response(iirfilters)



