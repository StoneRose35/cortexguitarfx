import numpy as np


def compute_feedback(tau, d_samples, f_sampling=48000):
    return np.power(0.001, d_samples/tau/f_sampling)


def get_primes_fct(n_max):
    primes = []
    for i in range(2,n_max):
        is_prime = True
        for k in range(2,i):
            if i%k==0:
                is_prime = False
                break
        if is_prime:
            primes.append(i)
    return primes     
        

''' computes delay prime number of n_delays by placing the prime number in bin of equal space
'''
def compute_delaytimes(f_sample=48000,n_max=4096,n_delays=4,max_delay_time=15/300, min_delay_time= 5/300):
    output = "const uint16_t delayInSamples[4] = {{{}}};"
    delay_bins = np.linspace(min_delay_time, max_delay_time, n_delays+1)*f_sample
    primenumbers = get_primes_fct(n_max=n_max)
    c=0
    delay_vals = []
    while c < n_delays:
        f_primes = list(filter(lambda x: delay_bins[c+1] > x > delay_bins[c],primenumbers))
        if len(f_primes) == 0:
            print("no prime number in bin [{:.0f};{:.0f}]".format(delay_bins[c],delay_bins[c+1]))
        elif f_primes[0] > n_max:
            print("delay time {} exceeds the maximum delay line length {}".format(f_primes[0], n_max))
        else:
            delay_vals.append(f_primes[0])
        c += 1
    delay_array = ""
    if len(delay_vals)==n_delays:
        for q in range(n_delays):
            delay_array += "{}, ".format(delay_vals[q])
    print(output.format(delay_array))
    return delay_vals


def compute_taus(delay_times,n_delays=4,tau_min=0.1,tau_max=2.0):
    taus = np.linspace(tau_min,tau_max,n_delays)
    output = "const int16_t feedback[4][4] = {\r\n"
    for dt in delay_times:
        output += "{\r\n"
        for t in taus:
            fbk = compute_feedback(t,dt)
            output += "0x{:x}, ".format(int(fbk*32767))
        output += "\r\n},"
    output += "};"
    print(output)
    #print("feedback for {}s reverb and unit delay of {:.3f}ms: {}".format(tau,d_samples*1000.0/f_sampling, feedbk))


if __name__ == "__main__":
    f_sample = 48000
    n_delays = 4
    delayline_length = 4096
    delay_time_min = 1.5/300
    delay_time_max = 8/300
    tau_min = 0.1
    tau_max = 2.0
    print("min delay time {:.1f}ms, max delay time {:.1f}ms".format(delay_time_min*1000.0, delay_time_max*1000.0))
    print("computing delay times for the main delay section")
    delaytimes = compute_delaytimes(f_sample=f_sample,n_max=delayline_length,n_delays=n_delays,
                        max_delay_time=delay_time_max,min_delay_time=delay_time_min)
    if len(delaytimes) == n_delays:
        compute_taus(delaytimes,n_delays=n_delays, tau_min=tau_min, tau_max=tau_max)
    
    print("computing delay lengths for the diffuser")
    delay_time_min = 2/1000
    delay_time_max = 54/1000
    n_delays = 4
    delayline_length = 2048
    print("min delay time {:.1f}ms, max delay time {:.1f}ms".format(delay_time_min*1000.0, delay_time_max*1000.0))
    delaytimes = compute_delaytimes(f_sample=f_sample,n_max=delayline_length,n_delays=n_delays,
                        max_delay_time=delay_time_max,min_delay_time=delay_time_min)


