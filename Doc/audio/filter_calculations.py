import numpy as np
import scipy.signal
import scipy.signal as signal
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
import math
import cmath


class AudioFilter:
    def __init__(self,a,b):
        self.a=np.array(a,dtype="float32")
        self.b=np.array(b,dtype="float32")
        self.w=np.array([0,0,0],dtype="float32")

    def apply(self, sample):
        self.w[0] = np.float32(sample) - self.a[0]*self.w[1] - self.a[1]*self.w[2]
        out = self.b[0]*self.w[0] + self.b[1]*self.w[1] + self.b[2]*self.w[2]
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

def generate_c_declaration(sos,fname="filter1"):
    templatestring = """.{fname} = 
{{
    .coeffA = {{{a1:.8f}f, {a2:.8f}f}},
    .coeffB = {{{b0:.8f}f, {b1:.8f}f, {b2:.8f}f}},
    .w = {{0.0f,0.0f,0.0f}}
}},"""
    return templatestring.format(fname=fname,a1=sos[4],a2=sos[5],b0=sos[0],b1=sos[1],b2=sos[2])

def design_iir_filter(rs=20,fc=None,fs=48000,type="cheby2",ftype="lowpass"):
    if fc is None:
        fc = fs/2
    if ftype == "highpass" or ftype=="lowpass":
        order = 2
    else:
        order = 1
    if type == "cheby2":
        sos = signal.cheby2(order,rs, fc, analog=False, btype=ftype, output='sos', fs=fs)
    elif type == "cheby1":
        sos = signal.cheby1(order, rs, fc, analog=False, btype=ftype, output='sos', fs=fs)
    elif type == "butter":
        sos = signal.butter(order, fc, analog=False, btype=ftype, output='sos', fs=fs)
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

        bvals = np.array(bd).astype(float)
        avals = np.array(ad[1:]).astype(float)
        # pole calculation
        p1 = (-float(avals[0]) + cmath.sqrt(float(avals[0])*float(avals[0]) - 4*float(avals[1])))/(2.)
        p2 = (-float(avals[0]) - cmath.sqrt(float(avals[0]) * float(avals[0]) - 4*float(avals[1]))) /(2.)
        z1 = (-float(bvals[1]) + cmath.sqrt(float(bvals[1])*float(bvals[1]) - 4*float(bvals[0])*float(bvals[2])))/(2.)
        z2 = (-float(bvals[1]) - cmath.sqrt(float(bvals[1]) * float(bvals[1]) - 4 * float(bvals[0]) * float(bvals[2]))) /(2.)

        z,p,k = scipy.signal.tf2zpk(bd,ad)

        if do_plot is True:
            print("pole 1 at {:.3f}/{:.3f}, absolute value: {:.3f}".format(np.real(p1), np.imag(p1), abs(p1)))
            print("pole 2 at {:.3f}/{:.3f}, absolute value: {:.3f}".format(np.real(p2), np.imag(p2), abs(p2)))
            print("zero 1 at {:.3f}/{:.3f}".format(np.real(z1), np.imag(z1)))
            print("zero 2 at {:.3f}/{:.3f}".format(np.real(z2), np.imag(z2)))
            print("gain: {:.3f}".format(k))
            print(generate_c_declaration(second_order_filter))
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
            testFilter = AudioFilter(avals,bvals)
            tdOutput=[]
            for c in range(1024):
                if c==0:
                    inputVal=1.
                else:
                    inputVal = 0
                tdOutput.append(testFilter.apply(inputVal))
            axs2=fig.add_subplot(gs[2,0])
            axs2.plot(x_time_domain,tdOutput, ".-r", label="positive pulse")
            testFilter = AudioFilter(avals,bvals)
            tdOutput=[]
            for c in range(1024):
                if c==0:
                    inputVal=-1.0
                else:
                    inputVal = 0
                tdOutput.append(testFilter.apply(inputVal))
            axs2.plot(x_time_domain,tdOutput, ".-g", label="negative pulse")
            axs2.set_ylim([-1.007, 1.007])
            axs2.set_yticks(np.arange(start=-1.0,stop=1.0,step=0.25))
            axs2.set_title("Time Domain response")
            axs2.set_xlabel("Time [ms]")
            axs2.set_ylabel("Ampl. [Float]")
            axs2.grid(color="gray", linestyle="--", which="both", axis="y")
            axs2.legend(loc="upper right")

            testFilter = AudioFilter(avals, bvals)
            tdOutput=[]
            for c in range(1024):
                inputVal = (np.random.random()-0.5)*2.
                try:
                    tdOutput.append(testFilter.apply(inputVal))
                except OverflowError:
                    tdOutput.append(0)
            axs4 = fig.add_subplot(gs[2,1])
            axs4.plot(x_time_domain,tdOutput, ".-g", label="negative pulse")
            axs4.set_ylim([-1.007, 1.007])
            axs4.set_yticks(np.arange(start=-1.0,stop=1.0,step=0.25))
            axs4.set_title("Noise response")
            axs4.set_xlabel("Time [ms]")
            axs4.set_ylabel("Ampl. [Float]")
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

def design_and_plot_iir_filter(do_plot=True,rs=20,fc=None,fs=48000,do_overflow=True,type="cheby2",ftype="lowpass"):
    sos = design_iir_filter(rs=rs,fc=fc,fs=fs,type=type,ftype=ftype)
    plot_iir_filter(sos,do_plot=do_plot,fs=fs,do_overflow=do_overflow,fc=fc, ftype=ftype,type=type,rs=rs)
    return sos

def plot_sos_frequency_response(sosections,fs=48000,nfreqs=512):

    h_tot = np.ones(nfreqs)
    w = np.logspace(np.log10(20.),np.log10(fs/2.),nfreqs)
    for sos in sosections:
        b = sos[0]
        a = sos[1]
        wnew, h = scipy.signal.freqz(b, a, worN=w, fs=fs)
        h_tot = h_tot * h

    fig, axxes = plt.subplots(2, 1)
    axxes[0].plot(w, 20. * np.log10(abs(h_tot)) - max(20.* np.log10(abs(h_tot))), "-r")
    axxes[0].set_xscale("log")
    axxes[0].set_ylim([-60,0])

    axxes[1].plot(w, np.unwrap(np.arctan2(np.real(h_tot), np.imag(h_tot))), "-r")
    axxes[1].set_xscale("log")
    plt.show()



if __name__ == "__main__":

    # manual modeling attempt of a guitar speaker using 4 iir filters
    iirfilters=[]
    sos = design_and_plot_iir_filter(True,rs=6,fc=3500,type="cheby1",ftype="lowpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])
    sos = design_and_plot_iir_filter(True, rs=6, fc=2500, type="cheby1", ftype="lowpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])
    sos = design_and_plot_iir_filter(True, rs=16, fc=120, type="cheby1", ftype="highpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])
    sos = design_and_plot_iir_filter(True, rs=1, fc=300, type="butter", ftype="highpass")
    iirfilters.append([sos[0][:3],sos[0][3:]])

    plot_sos_frequency_response(iirfilters)



