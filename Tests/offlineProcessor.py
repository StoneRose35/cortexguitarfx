
import tkinter as tk
from tkinter import ttk, font
import os
import subprocess
import json


class OfflineProcessorGui:
    def __init__(self):
        self.w = tk.Tk()
        self.sliders = []
        self.labels = []
        self.currentProgram = -1
        bigfont = font.Font(family="helvetica", size=20)
        self.w.option_add("*Font", bigfont)
        self.w.title("PiPicoFX Offline Processor")
        self.w.geometry("760x1000")

        # look for all audio samples and put them in a combobox
        cbSamples = ttk.Combobox(self.w, width=38)
        sampleVals = []
        for el in os.listdir("./audiosamples"):
            if not el.endswith("_proc.wav"):
                dname = el.replace(".wav", "")
                sampleVals.append(dname)
        cbSamples["values"] = sampleVals
        cbSamples.grid(pady=10, padx=10, sticky="w")

        # list all fx programs and put them into a combobox
        fxPrograms = call_backed(["-o"])
        cbPrograms = ttk.Combobox(self.w, width=38)
        cbPrograms["values"] = fxPrograms["programs"]
        cbPrograms.grid(pady=10, padx=10, sticky="w")
        cbPrograms.bind("<<ComboboxSelected>>", self.fxprogram_changed)

        self.param_frame = tk.Frame(self.w)
        self.param_frame.columnconfigure(0,weight=1, minsize=120)
        self.param_frame.columnconfigure(1,weight=3)
        self.param_frame.grid(sticky="w")

        self.w.mainloop()

    def fxprogram_changed(self, event):

        self.currentProgram = event.widget.current()
        params = call_backed(["-n", str(self.currentProgram)])

        # clear slider and add new ones
        for s in self.sliders:
            s.grid_remove()
        for l in self.labels:
            l.grid_remove()
        self.sliders = []
        self.labels = []
        cnt = 0
        self.programValues = []
        for pn in params["parameterNames"]:
            lbl = tk.Label(self.param_frame, text=pn["name"],width=20,justify="left",anchor="w")
            lbl.grid(column=0, row=cnt,sticky="sw",padx=10)
            self.labels.append(lbl)
            self.programValues.append(tk.StringVar(self.param_frame))
            val = tk.Label(self.param_frame, text=" ", textvariable=self.programValues[-1])
            val.grid(column=0, row=cnt+1,sticky="nw",padx=10)
            self.labels.append(val)
            scl = tk.Scale(self.param_frame, orient="horizontal", from_=0, to=4095, command=self.param_changed, width=30, length=400)
            scl.grid(column=1, row=cnt, rowspan=2,pady=16)
            self.sliders.append(scl)

            cnt +=2
        pass

    def param_changed(self,event):
        args=["-v",str(self.currentProgram) ]
        for s in self.sliders:
            args.append(str(int(s.get())))
        currentVals = call_backed(args)
        for v in zip(currentVals["parameterValues"],self.programValues):
            v[1].set(v[0]["value"])


        pass

def call_backed(arguments):
    cmd = ["./processThroughFxProgram"]
    for arg in arguments:
        cmd.append(arg)
    outb = subprocess.check_output(cmd)
    try:
        jobj = json.loads(outb.decode("utf-8"))
        return jobj
    except:
        return {"error": outb.decode("utf-8")}






if __name__ == "__main__":
    OfflineProcessorGui()
