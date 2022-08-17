import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt
import math

class AudioFilter:
    def __init__(self,a,b,sample_size):
        self.a=a
        self.b=b
        self.sample_size = sample_size
        self.bfrIn = np.zeros(len(b))
        #self.bfrOut = np.zeros(len(a))

    def apply(self, sample):
        out = 0
        for cnt in range(len(self.bfrIn)-1,0,-1):
            self.bfrIn[cnt] = self.bfrIn[cnt-1]
        self.bfrIn[0] = sample

        for c in range(1, len(self.bfrIn)):
            self.bfrIn[0] -= self.a[c]*self.bfrIn[c]/(2**(self.sample_size))
        for c in range(len(self.bfrIn)):
            out = out + self.b[c]*self.bfrIn[c]/(2**(self.sample_size)) # - self.a[c]*self.bfrOut[c]/(2**(self.sample_size-1))
        out = int(out)
        #for cnt in range(len(self.bfrOut)-1,0,-1):
        #    self.bfrOut[cnt] = self.bfrOut[cnt-1]
        #self.bfrOut[0] = out
        return out

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

class SawtoothAdditive:
    def __init__(self):
        self.currentPhase = 0
        self.sinetable = get_sine_table()

    def getSine(self,phaseinc):
        self.currentPhase += phaseinc
        idx1 = int((self.currentPhase >> 24) & 0xFF)
        idx2 = (idx1 + 1) & 0xFF
        idxrem = (self.currentPhase >> 8) & 0xFFFF
        v1 = self.sinetable[idx1]
        v2 = self.sinetable[idx2]
        sineval =v1 + (int((v2-v1)*idxrem)/(1 << 16))

        self.currentPhase += phaseinc
        self.currentPhase &= 0xFFFFFFFF
        return sineval

def design_and_plot_oversampling_lowpass_cheby(do_plot=False,to_integer=True,oversampling=2):
    ftype = 'lowpass'
    fs=48000
    fc = fs/2
    order = 2
    sample_size = 16
    rs=20
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

def design_and_plot_oversampling_lowpass_butter(do_plot=False,to_integer=True,oversampling=2):
    ftype = 'lowpass'
    fs=48000
    fc = fs/2
    order = 2
    sample_size = 16

    f0 = fc/(fs*oversampling)*2.
    bd, ad = signal.butter(order, f0, analog=False, btype=ftype, output='ba')

    bvals = np.array(bd * ((1 << (sample_size)) - 1)).astype(int)
    avals = np.array(ad*((1 << (sample_size)) -1)).astype(int)
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

def design_and_plot_oversampling_lowpass_fir(do_plot=False, to_integer=True, oversampling=2):
    firsize = 4
    firwidth = 0.1
    fs=48000
    sample_size = 16
    bd = signal.firwin(firsize, 1 / oversampling, firwidth)
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

def antialiasing_filter():
    notenr=126
    phaseincr_bitsize=32
    sample_size=16
    oversampling=4
    sample_rate = 48000
    n_waveforms=32
    phaseincs = get_phase_increments(sample_rate, oversampling=oversampling, bitres=phaseincr_bitsize)
    nplots = n_waveforms*int((1 << phaseincr_bitsize) /  phaseincs[notenr - 1])

    a_vals, b_vals = design_and_plot_oversampling_lowpass_fir(True,True,oversampling)

    oversampling_filter = AudioFilter(a_vals, b_vals, sample_size)
    waveform_raw = []
    waveform_filt = []
    current_phase = 0
    for c in range(nplots):
        phase_bkp = current_phase
        #current_phase += (phaseincs[notenr-1] & 0xFFFFFFFF)
        #waveformval = get_saw(current_phase)
        #waveform_raw.append(waveformval)
        #waveform_filt.append(oversampling_filter.apply(waveformval))
        os_buffer_raw = []
        os_buffer_filt = []
        for i in range(oversampling):
            current_phase += (phaseincs[notenr - 1] & 0xFFFFFFFF)
            waveformval = get_saw(current_phase)
            waveform_raw.append(waveformval)
        current_phase = phase_bkp
        for i in range(oversampling):
            current_phase += (phaseincs[notenr - 1] & 0xFFFFFFFF)
            waveformval = get_saw(current_phase)
            waveform_filt.append(oversampling_filter.apply(waveformval))

    downsampled_raw=[]
    downsampled_filtered=[]
    for q in range(int(len(waveform_raw)/oversampling)):
        downsampled_raw.append(np.average(waveform_raw[q*oversampling:(q+1)*oversampling]))
        downsampled_filtered.append(np.average(waveform_filt[q*oversampling:(q+1)*oversampling]))

    #downsampled_raw = waveform_raw[::oversampling]
    #downsampled_filtered = waveform_filt[::oversampling]

    plt.subplot(2,1,1)
    plt.plot(waveform_raw,"r")
    plt.plot(waveform_filt,"b")
    plt.subplot(2,1,2)
    plt.plot(downsampled_raw,"r")
    plt.plot(downsampled_filtered,"b")
    plt.show()


if __name__ == "__main__":
    notenr = 128
    sample_rate=48000
    oversampling = 1
    phaseincr_bitsize=32
    n_waveforms=32
    phaseincs, f_synths = get_phase_increments(sample_rate, oversampling=oversampling, bitres=phaseincr_bitsize)
    phaseinc= phaseincs[notenr-1]
    nplots = n_waveforms*int((1 << phaseincr_bitsize) /  phaseinc)
    sawtoothOscillator = SawtoothAdditive()
    phaseincs = get_phase_increments(sample_rate, oversampling=oversampling, bitres=phaseincr_bitsize)

    waveform = []
    for c in range(nplots):
        waveform.append(sawtoothOscillator.getSine(phaseinc))
    plt.plot(waveform,"-k")
    plt.title("Freq: {:.1f}".format(f_synths[notenr-1]))
    plt.show()



