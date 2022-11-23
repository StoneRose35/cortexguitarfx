import scipy.io
import scipy.io.wavfile
import scipy.fft
import scipy.interpolate
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal
import scipy.optimize
import audio.filter_calculations
import audio.ir_optimizer


def get_ir(fname,sampling_rate=48000):
    wavdata = scipy.io.wavfile.read(fname)
    try:
        if len(wavdata[1][0]) > 1:
            rawwav = np.array(list(map(lambda x: x[0],wavdata[1])))
        else:
            rawwav = wavdata[1]
    except:
        rawwav = wavdata[1]
    powertwo=2
    dt = 1./wavdata[0]
    dt_expected=1./sampling_rate
    if (np.abs(dt-dt_expected) > 1e-9):
        n_samples_new = int(np.ceil(len(rawwav)*dt/dt_expected))
        x_old = np.linspace(0,len(rawwav)*dt,len(rawwav))
        x_new = np.linspace(0,len(rawwav)*dt,n_samples_new)
        interpolator = scipy.interpolate.interp1d (x_old,rawwav)
        rawwav = interpolator(x_new)
    while (1 << powertwo) < rawwav.size:
        powertwo += 1
    paddedsize = (1 << powertwo)
    rawwav = rawwav/32767
    arr_padded = np.pad(rawwav, (0, paddedsize-rawwav.size), mode='constant', constant_values=0)
    return arr_padded

def shorten_ir(ir, nsamples=256,slope_length=16):

    windowfct=np.ones(nsamples-slope_length)
    windowfct = np.pad(windowfct,(0,slope_length),mode='linear_ramp',end_values=(0,0))
    windowfct = np.pad(windowfct,(0,ir.size-windowfct.size),"constant",constant_values=0)
    arr_padded = ir*windowfct
    return arr_padded[:nsamples]


def highpassed_ir(ir, f_cutoff,f_sample):
    spectr = scipy.fft.fft(ir)
    max_spectr = max(abs(spectr))

    f_scale = scipy.fft.fftfreq(ir.size,1./f_sample)
    for i in range(ir.size):
        if (f_scale[i] >= 0 and f_scale[i] < f_cutoff) or (f_scale[i] < 0 and f_scale[i] > -f_cutoff):
            spectr[i] = max_spectr
    hp_ir = np.real(scipy.fft.ifft(spectr))

    return hp_ir

def interpolate_spectrum(filename,f_bins=64,f_sample=48000):
    ir = get_ir(filename)
    spectr = scipy.fft.fft(ir)
    freqs = scipy.fft.fftfreq(len(ir),1./f_sample)
    freqs_to_interpolate = scipy.fft.fftfreq(f_bins,1./f_sample)
    spectr_r = np.real(spectr)
    spectr_i = np.imag(spectr)
    interpolator_r = scipy.interpolate.interp1d(freqs,spectr_r)
    interpolator_i = scipy.interpolate.interp1d(freqs,spectr_i)
    spectr_interpolated_r = interpolator_r(freqs_to_interpolate)
    spectr_interpolated_i = interpolator_i(freqs_to_interpolate)
    spectr_interpolated_complex = []
    for e in zip(spectr_interpolated_r,spectr_interpolated_i):
        spectr_interpolated_complex.append(complex(e[0],e[1]))
    ir_interpolated = scipy.fft.ifft(spectr_interpolated_complex)
    plt.plot(np.real(ir_interpolated), '-b')
    plt.plot(np.imag(ir_interpolated), '-r')
    plt.show()




def renorm_ir(ir):
    current_power = np.sum(ir)# np.sqrt(np.sum(np.power(ir,2)))
    return ir/current_power

def plot_model_cab_curve(sampling_rate=44100,axes=None,style="-k",sample_length=4096,floattype=False):
    hz = np.ones(sample_length)
    iir_lowpass_order = 2
    iir_lowpass_cutoff = 6000
    b, a=scipy.signal.butter(iir_lowpass_order,iir_lowpass_cutoff,btype="low",analog=False,output="ba",fs=sampling_rate)
    b_out, a_out = scipy.signal.butter(iir_lowpass_order,iir_lowpass_cutoff,btype="low",analog=False,output="ba",fs=sampling_rate)
    b_out = b_out*32767.
    a_out = a_out*32767.
    if floattype is False:
        print("butterworth lowpass @{}Hz,\r\n\tb: {}\r\n\ta: {}".format(iir_lowpass_cutoff,b_out.astype("int16"),a_out.astype("int16")))
    else:
        print("butterworth lowpass @{}Hz,\r\n\tb: {}\r\n\ta: {}".format(iir_lowpass_cutoff, b.astype("float"),
                                                                        a.astype("float")))
    wz, hzn = scipy.signal.freqz(b,a,fs=sampling_rate,worN=sample_length)
    hz = hz*hzn

    iir_highpass_cutoff = 20
    iir_highpass_order = 2
    iir_highpass_atten = 20
    b, a = scipy.signal.cheby2(iir_highpass_order,iir_highpass_atten,iir_highpass_cutoff,btype="high",analog=False,output="ba",fs=sampling_rate)
    b_out, a_out = scipy.signal.cheby2(iir_highpass_order, iir_highpass_atten, iir_highpass_cutoff, btype="high", analog=False,
                               output="ba", fs=sampling_rate)
    b_out = b_out*32767.
    a_out = a_out*32767.
    if floattype is False:
        print("chebychev highpass @{}Hz with attenuation {}dB\r\n\tb: {}\r\n\ta: {}".format(iir_highpass_cutoff, iir_highpass_atten, b_out.astype("int16"), a_out.astype("int16")))
    else:
        print("chebychev highpass @{}Hz with attenuation {}dB\r\n\tb: {}\r\n\ta: {}".format(iir_highpass_cutoff, iir_highpass_atten, b.astype("float"), a.astype("float")))
    #b, a = scipy.signal.butter(iir_highpass_order,iir_highpass_cutoff,btype="high",analog=False,output="ba",fs=sampling_rate)
    wzb,hzn = scipy.signal.freqz(b,a,fs=sampling_rate,worN=sample_length)
    #hz = hz*hzn
    freqs = wz

    low_peak_freq = 200
    low_peak_width = 140
    low_peak_atten=10
    low_peak_order = 2
    low_peak_mix = 0.0
    [b,a ] = scipy.signal.cheby2(low_peak_order,low_peak_atten,[low_peak_freq-low_peak_width,low_peak_freq+low_peak_width],btype="bandpass",analog=False,output="ba",fs=sampling_rate)
    wzb, hzn = scipy.signal.freqz(b,a,fs=sampling_rate,worN=sample_length)
    hzn = hzn / max(abs(hzn))
    hz = hz*(1.-low_peak_mix) + hzn*low_peak_mix

    ir_size=64
    ir_pad_size=8
    ir_highpass_freq = 100
    ir = get_ir("resources/TubePreamp2/DYN-7B/OD-E112-G12-65-DYN-7B-09-30-BRIGHT.wav")
    #ir = highpassed_ir(ir,ir_highpass_freq,sampling_rate)
    short_ir = shorten_ir(ir,ir_size,ir_pad_size)
    short_ir = highpassed_ir(short_ir,ir_highpass_freq,sampling_rate)
    short_ir = renorm_ir(short_ir)
    wzb, hzn = scipy.signal.freqz(short_ir, fs=sampling_rate,worN=sample_length)
    for el in short_ir:
        if floattype is False:
            print("0x{:x}, ".format(np.ushort(el*32767)),end="")
        else:
            print("{:f}f, ".format(el), end="")
    hzn=hzn/max(abs(hzn))
    hz = hz*hzn
    if axes is None:
        plt.plot(freqs,20.*np.log10(abs(hz)),style)
    else:
        axes.plot(freqs,20.*np.log10(abs(hz)),style)

def plot_freq_response(fname,axes=None,style=None):
    wavdata =scipy.io.wavfile.read(fname)
    if len(wavdata[1][0]) > 1:
        rawwav = np.array(list(map(lambda x: x[0],wavdata[1])))
    else:
        rawwav = wavdata[1]
    powertwo=2
    dt = 1./wavdata[0]
    while (1 << powertwo) < rawwav.size:
        powertwo += 1
    paddedsize = (1 << powertwo)
    arr_padded = np.pad(rawwav,(0,paddedsize-rawwav.size),mode='constant',constant_values=0)
    spec = scipy.fft.fft(arr_padded)
    logspec = 20. * np.log10(np.abs(spec[:int(paddedsize / 2)]))
    maxlog = np.max(logspec)
    faxis = scipy.fft.fftfreq(paddedsize,dt)
    if style is None:
        style = "-k"
    if axes is None:
        plt.plot(faxis[:int(paddedsize/2)],logspec - maxlog,style)
        plt.xscale("log")
        plt.show()
    else:
        axes.plot(faxis[:int(paddedsize/2)],logspec - maxlog,style)
        axes.set_xscale("log")
    pass

def plot_ir(fname,axes=None,style=None):
    wavdata =scipy.io.wavfile.read(fname)
    if len(wavdata[1][0]) > 1:
        rawwav = np.array(list(map(lambda x: x[0],wavdata[1])))
    else:
        rawwav = wavdata[1]

    dt = 1./wavdata[0]
    taxis = np.linspace(0,dt*rawwav.size,rawwav.size)
    if style is None:
        style = "-k"
    if axes is None:
        plt.plot(taxis,rawwav,style)
        plt.show()
    else:
        axes.plot(taxis,rawwav,style)
        #axes.set_xscale("log")
    pass

def plot_simple_model_cab_curve(axes=None,style="-k"):
    hz = np.ones(512)
    iir_lowpass_order = 2
    iir_lowpass_cutoff = 4200
    b, a = scipy.signal.butter(iir_lowpass_order, iir_lowpass_cutoff, btype="low", analog=False, output="ba",
                                       fs=48000)
    b_out, a_out = scipy.signal.butter(iir_lowpass_order, iir_lowpass_cutoff, btype="low", analog=False, output="ba",
                                       fs=48000)
    b_out = b_out * 32767.
    a_out = a_out * 32767.
    print("butterworth lowpass @{}Hz,\r\n\tb: {}\r\n\ta: {}".format(iir_lowpass_cutoff, b_out.astype("int32"),
                                                                    a_out.astype("int32")))
    wz, hzn = scipy.signal.freqz(b, a, fs=48000, worN=512)
    hz = hz * hzn

    iir_highpass_cutoff = 80
    iir_highpass_order = 2
    b, a = scipy.signal.butter(iir_highpass_order, iir_highpass_cutoff, btype="high", analog=False,
                               output="ba", fs=48000)
    b_out, a_out = scipy.signal.butter(iir_highpass_order, iir_highpass_cutoff, btype="high",
                                       analog=False,
                                       output="ba", fs=48000)
    b_out = b_out * 32767.
    a_out = a_out * 32767.
    print("butterworth highpass @{}Hz \r\n\tb: {}\r\n\ta: {}".format(iir_highpass_cutoff,
                                                                                        b_out.astype("int32"),
                                                                                        a_out.astype("int32")))
    wzb, hzn = scipy.signal.freqz(b, a, fs=48000, worN=512)
    hz = hz * hzn

    # midcut
    midcut_freqs = [80, 200]
    midcut_order = 1
    midcut_atten = 10
    midcut_fact=0.98
    b,a = scipy.signal.cheby1(midcut_order,midcut_atten,midcut_freqs,btype="bandstop",analog=False,output="ba",fs=48000)
    #b=-b
    b[0]=(1.0-midcut_fact) + b[0]*midcut_fact
    b[1]=b[1]*midcut_fact
    b[2]=b[2]*midcut_fact
    a[1]=a[1]*midcut_fact
    a[2]=a[2]*midcut_fact
    b_out = b* 32767.
    a_out = a* 32767.
    wzb, hzn = scipy.signal.freqz(b, a, fs=48000, worN=512)
    #hzn = 1 - hzn*midcut_fact
    print("chebychev type 1 midcut @{}Hz, ripple: {}\r\n\tb: {}\r\n\ta: {}".format(midcut_freqs, midcut_atten,
                                                                                        b_out.astype("int32"),
                                                                                        a_out.astype("int32")))

    hz = hz * hzn
    freqs = wz


    if axes is None:
        plt.plot(freqs, 20. * np.log10(abs(hz)), style)
    else:
        axes.plot(freqs, 20. * np.log10(abs(hz)), style)



if __name__ == "__main__":

    # generates data for amp model 1 (manually tuned cab consisiting of an fir filter in combination with an iir filter
    #plot_model_cab_curve(sampling_rate=48000, axes=None, style="-k", sample_length=4096,floattype=True)
    #plt.show()

    # generates data for amp model 2
    # vary cab_index to compute various cabs
    cab_index = 1
    ir_files = ["resources/soundwoofer/Hiwatt Maxwatt M412 SM57 2.wav", "resources/soundwoofer/Fender Frontman 212 AKG D112.wav", "resources/soundwoofer/Vox AC15C1 SM57 1.wav"]
    optimizer = audio.ir_optimizer.IrOptimizer(3)
    optimizer.load_ir(ir_files[cab_index])

    interpolate_spectrum(ir_files[cab_index])
    shortened_ir = optimizer.ir
    shortened_ir[64:] = 0
    shortened_ir[:64] = shorten_ir(np.array(optimizer.ir),64,48)
    #shortened_ir = np.array(shortened_ir)
    spec_shorted = scipy.fft.fft(shortened_ir)
    print("FIR Coefficients")
    shortened_ir = renorm_ir(shortened_ir)
    #max_ir = np.max(abs(shortened_ir))
    for cc in range(64):
        print("{}, ".format(int(shortened_ir[cc]*32767)), end="")
    print("\n\n")
    for cc in range(64):
        print("{}f, ".format(shortened_ir[cc]), end="")
    remaining_spec = np.array(optimizer.ir_spec_full)/spec_shorted
    halflen = int(len(spec_shorted)/2)
    plt.plot(20.*np.log10(abs(np.array(optimizer.ir_spec_full[:halflen]))),"-b")
    plt.plot(20.*np.log10(abs(np.array(spec_shorted[:halflen]))),"-g")
    plt.xscale("log")
    plt.show()


    res = scipy.optimize.minimize(optimizer.get_diff,optimizer.optim_data,method='Nelder-Mead',
                                  bounds=optimizer.bounds,callback=optimizer.iterator_callback,options = {"fatol":0.000001,'xatol': 0.000001})
    optimizer.iterator_callback(res.x,True)
    optimizer.optim_data = res.x
    best_sos = optimizer.get_sos()
    audio.filter_calculations.plot_iir_filter(best_sos,do_plot=True,fs=48000,do_overflow=True,sample_size=None,ftype="Custom",type="Custom")
