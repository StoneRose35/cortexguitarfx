import matplotlib.pyplot as plt
import csv

if __name__ == "__main__":
    signal_values = []
    with open("audioout.txt","rt") as f:
        rdr = csv.reader(f)
        for e in rdr:
            signal_values.append(int(e[0]))
    plt.plot(signal_values,"+-k")
    plt.show()