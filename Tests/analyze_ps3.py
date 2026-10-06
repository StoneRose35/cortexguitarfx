import csv
import math
import matplotlib.pyplot as plt
if __name__ == "__main__":
    with open("ps3.csv","rt") as f:
        rdr = csv.reader(f)
        data_proc=[[],[],[],[]]
        cnt=0
        for row in rdr:
            if (cnt > 0):
                distance_proc = int(row[3]) - round(int(row[3])/(48000.0/330.0))*(48000.0/330.0)
                data_proc[0].append(int(row[0]))
                data_proc[1].append(int(row[1]))
                data_proc[2].append(int(row[2]))
                data_proc[3].append(distance_proc)
            cnt+=1
        pass
        plt.plot(data_proc[0],data_proc[3])
        plt.show()