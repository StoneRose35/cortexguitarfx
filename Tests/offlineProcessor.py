
import tkinter as tk
from tkinter import ttk, font
import os
import subprocess
import json


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

def param_changed(event):
    pass

def fxprogram_changed(event):
    global w
    global sliders
    idx = event.widget.current()
    params = call_backed(["-n", str(idx)])

    # clear slider and add new ones
    for s in sliders:
        s.pack_forget()
    sliders = []
    for pn in params["parameterNames"]:
        lbl = tk.Label(w, text=pn["name"], anchor="w")
        lbl.pack()
        sliders.append(lbl)
        scl = tk.Scale(w, orient="horizontal", from_=0, to=4095, command=param_changed,width=30,length=400)
        scl.pack()
        sliders.append(scl)
    pass


if __name__ == "__main__":
    w = tk.Tk()
    sliders=[]
    fontfamilies = font.families()
    bigfont = font.Font(family="helvetica", size=20)
    w.option_add("*Font", bigfont)
    w.title("PiPicoFX Offline Processor")

    # look for all audio samples and put them in a combobox
    cbSamples = ttk.Combobox(w,width=40,heigh=30)
    sampleVals = []
    for el in os.listdir("./audiosamples"):
        if not el.endswith("_proc.wav"):
            dname = el.replace(".wav", "")
            sampleVals.append(dname)
    cbSamples["values"] = sampleVals
    cbSamples.pack(pady=10, padx=10, anchor="w")

    # list all fx programs and put them into a combobox
    fxPrograms = call_backed(["-o"])
    cbPrograms = ttk.Combobox(w, width=40)
    cbPrograms["values"] = fxPrograms["programs"]
    cbPrograms.pack(pady=10, padx=10, anchor="w")
    cbPrograms.bind("<<ComboboxSelected>>", fxprogram_changed)
    w.mainloop()
