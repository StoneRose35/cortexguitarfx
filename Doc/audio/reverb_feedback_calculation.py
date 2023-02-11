import numpy as np

primenumbers = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101,
                103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199]


def compute_feedback(tau, d_samples, f_sampling=48000):
    return np.power(0.001, d_samples/tau/f_sampling)


if __name__ == "__main__":
    delay_times = [1549, 1553, 1559, 1567]#[31, 67, 101, 113]
    taus = np.linspace(0.1, 2.0, 4)
    tau = 0.1
    d_samples = 13
    f_sampling = 48000
    feedbk = compute_feedback(tau,d_samples)
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
