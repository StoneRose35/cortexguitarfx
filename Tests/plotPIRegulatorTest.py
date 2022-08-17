import matplotlib.pyplot as plt
import csv

if __name__ == "__main__":
    temps=[]
    heater=[]
    integral_factor=[]
    timevals=[]
    tcnt=0
    with open("./piregulator.txt","rt") as csvfile:
        rdr = csv.reader(csvfile,delimiter=",")
        for e in rdr:

            timevals.append(int(e[0])/60./60.)
            temps.append(float(e[1]))
            heater.append(int(e[2]))
            integral_factor.append(float(e[3]))
    plt.subplot(1,3,1)        
    plt.plot(timevals,temps)
    plt.subplot(1,3,2)
    plt.plot(timevals,heater)
    plt.subplot(1,3,3)
    plt.plot(timevals,integral_factor)
    plt.show()
