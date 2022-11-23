import numpy as np
import scipy.signal
import matplotlib.pyplot as plt


class IrOptimizer:
    def __init__(self,n_stages):
        self.optim_data = []
        self.bounds=[]
        self.bound_limit = 2.0
        for q in range(n_stages):
            self.optim_data += [1., 0., 0.,0. ,0.]
            self.bounds += [(-self.bound_limit,self.bound_limit),
                            (-self.bound_limit,self.bound_limit),
                            (-self.bound_limit,self.bound_limit),
                            (-self.bound_limit,self.bound_limit),
                            (-self.bound_limit,self.bound_limit)]
        self.optim_data = np.array(self.optim_data)
        self.ir_spec = []
        self.ir_spec_full =[]
        self.ir = []
        self.iteration_cntr = 0

    def get_and_resample_ir(self, fname, sampling_rate=48000):
        wavdata = scipy.io.wavfile.read(fname)
        try:
            if len(wavdata[1][0]) > 1:
                rawwav = np.array(list(map(lambda x: x[0], wavdata[1])))
            else:
                rawwav = wavdata[1]
        except:
            rawwav = wavdata[1]
        powertwo = 2
        dt = 1. / wavdata[0]
        dt_expected = 1. / sampling_rate
        if (np.abs(dt - dt_expected) > 1e-9):
            n_samples_new = int(np.ceil(len(rawwav) * dt / dt_expected))
            x_old = np.linspace(0, len(rawwav) * dt, len(rawwav))
            x_new = np.linspace(0, len(rawwav) * dt, n_samples_new)
            interpolator = scipy.interpolate.interp1d(x_old, rawwav)
            rawwav = interpolator(x_new)
        while (1 << powertwo) < rawwav.size:
            powertwo += 1
        paddedsize = (1 << powertwo)
        rawwav = rawwav / 32767
        arr_padded = np.pad(rawwav, (0, paddedsize - rawwav.size), mode='constant', constant_values=0)
        return arr_padded

    def load_ir(self,ir_file_name):
        ir = self.get_and_resample_ir(ir_file_name, 48000)
        # dt = 1./48000.
        halflen = int(len(ir) / 2.)

        ir_spec = scipy.fft.fft(ir)
        self.ir_spec_full = ir_spec
        ir_spec = ir_spec[:halflen]
        ir_spec = ir_spec / max(abs(ir_spec))
        self.ir_spec = ir_spec
        self.ir = ir

    def get_diff(self,opt_data):
        halflen = len(self.ir_spec)
        h_tot = np.ones(halflen)
        w = np.linspace(0, 48000. / 2., halflen)
        delta_gain = 0
        for idx in range(int(len(opt_data) / 5)):
            b = opt_data[idx*5:idx*5 + 3]
            a = np.array([1.])
            a = np.append(a,opt_data[idx*5+3:idx*5 + 5])
            w, h = scipy.signal.freqz(b, a, worN=halflen, fs=48000)
            h_tot = h_tot * h
            z, p, k = scipy.signal.tf2zpk(b, a)
            delta_gain += abs(1.-k)*4.
        #respdiff = map(lambda x, y: abs(x - y), self.ir_spec, h_tot)
        respdiff = map(lambda x, y: abs(abs(x) - abs(y)), self.ir_spec, h_tot)
        respdiff = sum(respdiff) + delta_gain

        return respdiff

    def get_modeled_spectrum(self):
        halflen = len(self.ir_spec)
        h_tot = np.ones(halflen)
        w = np.linspace(0, 48000. / 2., halflen)
        self.iteration_cntr += 1
        for idx in range(int(len(self.optim_data) / 5)):
            b = self.optim_data[idx*5:idx*5 + 3]
            a = np.array([1.])
            a = np.append(a,self.optim_data[idx*5+3:idx*5 + 5])
            w, h = scipy.signal.freqz(b, a, worN=halflen, fs=48000)
            h_tot = h_tot * h
        return np.concatenate((h_tot,np.flip(np.conj(h_tot))))

    def iterator_callback(self,current_vectr,do_plot=False):
        halflen = len(self.ir_spec)
        h_tot = np.ones(halflen)
        w = np.linspace(0, 48000. / 2., halflen)
        self.iteration_cntr += 1
        for idx in range(int(len(current_vectr) / 5)):
            b = current_vectr[idx*5:idx*5 + 3]
            a = np.array([1.])
            a = np.append(a,current_vectr[idx*5+3:idx*5 + 5])
            w, h = scipy.signal.freqz(b, a, worN=halflen, fs=48000)
            h_tot = h_tot * h
        logfreq = np.logspace(4, 14.287, len(w),base=2.0)
        interpolator_h_tot = scipy.interpolate.interp1d(w,h_tot)
        h_tot_logfreq = interpolator_h_tot(logfreq)
        interpolator_ir_spec = scipy.interpolate.interp1d(w, self.ir_spec)
        ir_spec_logfreq = interpolator_ir_spec(logfreq)
        #respdiff = map(lambda x, y: abs(x - y), self.ir_spec, h_tot)
        respdiff = map(lambda x, y: abs(np.log10(abs(x)) - np.log10(abs(y))), ir_spec_logfreq,h_tot_logfreq)
        respdiff = sum(respdiff)
        print("****** Iteration {} ******".format(self.iteration_cntr))
        for c in range(int(len(current_vectr)/5)):
            print("\tFilter {}".format(c+1))
            print("\t  b: [{:.6f}, {:.6f}, {:.6f}]".format(current_vectr[c*5],current_vectr[c*5+1],current_vectr[c*5+2]))
            print("\t  a: [{:.6f}, {:.6f}]".format(current_vectr[c*5+3],current_vectr[c*5+4]))
        print("difference: {:.3f}".format(respdiff))
        print("\r\n")
        if do_plot is True:
            fig, axxes = plt.subplots(2, 1)
            axxes[0].plot(w, 20. * np.log10(abs(self.ir_spec)), "-b")
            axxes[0].plot(w, 20. * np.log10(abs(h_tot)), ".-r")
            axxes[0].set_xscale("log")

            axxes[1].plot(w, np.unwrap(np.arctan2(np.real(self.ir_spec), np.imag(self.ir_spec))), "-b")
            axxes[1].plot(w, np.unwrap(np.arctan2(np.real(h_tot), np.imag(h_tot))), ".-r")
            axxes[1].set_xscale("log")
            plt.show()

    def get_sos(self):
        soss= []
        for idx in range(int(len(self.optim_data)/5)):
            sos = list(self.optim_data[idx*5:idx*5 + 3])
            sos.append(1.)
            sos += list(self.optim_data[idx*5+3:idx*5 + 5])
            soss.append(np.array(sos))
        return soss