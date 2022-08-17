import math

def get_phase_increments():
    notes = range(128)
    fs=48000
    bitres = 32
    for note in notes:
        notefreq = math.pow(2.,(note-64)/12.)*440.
        phaseinc = notefreq/fs*((1 << bitres))
        f_synth = int(phaseinc)*fs/((1 << bitres))
        if (phaseinc > (1 << (bitres-1))):
            print("note {} unavailable".format(note))
        else:
            #print("{},{}, {}, {}".format(note, notefreq, int(phaseinc),f_synth))
            print("{},".format(int(phaseinc)))

def get_sine_table():
    bitres = 16
    nvals = 256
    for c in range(nvals):
        print("{},".format(int(math.cos(c/nvals*math.pi*2)*((1 << (bitres-1)) - 1))))

if __name__ == "__main__":
    get_sine_table()    