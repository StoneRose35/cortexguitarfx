
import tkinter as tk
from tkinter import ttk, font
import os
import subprocess
import json
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
import pyaudio
import wave
import threading
import time


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
        self.w["bg"] = "#9bb0d1"

        self.w.grid_columnconfigure(0, weight=1)
        self.w.grid_columnconfigure(1, weight=1)
        self.w.grid_columnconfigure(2, weight=1)
        # look for all audio samples and put them in a combobox
        self.cbSamples = ttk.Combobox(self.w)
        sample_vals = []
        for el in os.listdir("./audiosamples"):
            if not el.endswith("_proc.wav"):
                dname = el.replace(".wav", "")
                sample_vals.append(dname)
        self.cbSamples["values"] = sample_vals
        self.cbSamples.grid(pady=10, padx=10, sticky="ew", columnspan=3)

        # list all fx programs and put them into a combobox
        fx_programs = call_backed(["-o"])
        cb_programs = ttk.Combobox(self.w)
        cb_programs["values"] = fx_programs["programs"]
        cb_programs.grid(pady=10, padx=10, sticky="ew", columnspan=3)
        cb_programs.bind("<<ComboboxSelected>>", self.fxprogram_changed)
        self.program_values = []
        self.param_frame = tk.Frame(self.w)
        self.param_frame.columnconfigure(0, weight=1, minsize=120)
        self.param_frame.columnconfigure(1, weight=3)
        self.param_frame.grid(sticky="ew", columnspan=3)
        self.param_frame["bg"] = "#bacce8"

        self.btnProcessSample = tk.Button(self.w, text="Process & Play", command=self.process_sample)
        self.btnProcessSample.grid(column=0, row=3, columnspan=1, sticky="ew", padx=15, pady=15)

        self.btnStopSample = tk.Button(self.w, text="Stop Audio", command=self.stop_audio)
        self.btnStopSample.grid(column=1, row=3, columnspan=1, sticky="ew", padx=15, pady=15)
        self.p = pyaudio.PyAudio()
        self.audioplayer_thread = threading.Thread(target=self.play_wav_file)
        self.audio_playing = False

        self.lblSampleTimeText = tk.StringVar(self.w)
        self.lblSampleTime = tk.Label(self.w, textvariable=self.lblSampleTimeText)
        self.lblSampleTime.grid(column=0, row=5, padx=15, pady=15, sticky="w")



        self.w.mainloop()

        # stop audio if playing
        if self.audioplayer_thread.is_alive():
            self.audio_playing = False
            while self.audioplayer_thread.is_alive():
                time.sleep(0.01)
        self.p.terminate()

    def fxprogram_changed(self, event):

        self.currentProgram = event.widget.current()
        params = call_backed(["-n", str(self.currentProgram)])

        # clear slider and add new ones
        for s in self.sliders:
            s.grid_remove()
        for label in self.labels:
            label.grid_remove()
        self.sliders = []
        self.labels = []
        cnt = 0

        for pn in params["parameterNames"]:
            lbl = tk.Label(self.param_frame, text=pn["name"], width=20, justify="left", anchor="w")
            lbl["bg"] = "#bacce8"
            lbl.grid(column=0, row=cnt, sticky="sw", padx=10)
            self.labels.append(lbl)
            self.program_values.append(tk.StringVar(self.param_frame))
            val = tk.Label(self.param_frame, text=" ", textvariable=self.program_values[-1])
            val["bg"] = "#bacce8"
            val.grid(column=0, row=cnt+1, sticky="nw", padx=10)
            self.labels.append(val)
            scl = tk.Scale(self.param_frame, orient="horizontal", from_=0, to=4095,
                           command=self.param_changed, width=30, length=400)
            scl.grid(column=1, row=cnt, rowspan=2, pady=16)
            scl["bg"] = "#bacce8"
            self.sliders.append(scl)

            cnt += 2
        pass

    def param_changed(self, event):
        args = ["-v", str(self.currentProgram)]
        for s in self.sliders:
            args.append(str(int(s.get())))
        current_vals = call_backed(args)
        for v in zip(current_vals["parameterValues"], self.program_values):
            v[1].set(v[0]["value"])
        pass

    def process_sample(self):
        samplename = "./audiosamples/" + self.cbSamples["values"][self.cbSamples.current()] + ".wav"
        samplename_proc = "./audiosamples/" + self.cbSamples["values"][self.cbSamples.current()] + "_proc.wav"
        if os.path.exists(samplename_proc):
            os.remove(samplename_proc)
        args = ["-p", samplename, str(self.currentProgram)]
        for s in self.sliders:
            args.append(str(int(s.get())))
        call_backed(args)
        wf = wave.open(samplename_proc, "rb")
        waveform = wf.readframes(-1)
        waveform = np.frombuffer(waveform, np.int16)

        figure = plt.Figure() #(figsize=(6, 5), dpi=100)
        ax = figure.add_subplot(111)
        ax.plot(waveform)
        ax.set_xticks([])
        ax.set_yticks([-32768, 0, 32767])
        ax.yaxis.grid(True, which="major")
        ax.set_frame_on(False)
        chart_type = FigureCanvasTkAgg(figure, self.w)
        chart_type.get_tk_widget().grid(sticky="we", columnspan=3, row=4, padx=15, pady=15)
        if os.path.exists(samplename_proc):
            if self.audioplayer_thread.is_alive():
                self.audio_playing = False
                while self.audioplayer_thread.is_alive():
                    time.sleep(0.01)
            self.audioplayer_thread = threading.Thread(target=self.play_wav_file, args=[samplename_proc])
            self.audioplayer_thread.start()

    def stop_audio(self):
        self.audio_playing = False

    def play_wav_file(self, wavfile):
        wf = wave.open(wavfile, "rb")
        self.audio_playing = True
        stream = self.p.open(format=self.p.get_format_from_width(wf.getsampwidth()),
                             channels=wf.getnchannels(),
                             rate=wf.getframerate(),
                             output=True)
        tottime = wf.getnframes()/wf.getframerate()
        timeplayed=0
        # Play samples from the wave file (3)
        while len(data := wf.readframes(1024)) and self.audio_playing is True:
            stream.write(data)
            timeplayed += len(data)
            time_displ = timeplayed/wf.getframerate()/wf.getnchannels()/wf.getsampwidth()
            self.lblSampleTimeText.set("{:.1f}/{:.1f}".format(time_displ, tottime))
        # Close stream (4)
        stream.close()
        wf.close()


def call_backed(arguments):
    cmd = ["./processThroughFxProgram"]
    for arg in arguments:
        cmd.append(arg)
    outb = subprocess.check_output(cmd)
    try:
        jobj = json.loads(outb.decode("utf-8"))
        return jobj
    except json.JSONDecodeError:
        return json.loads('{"error": outb.decode("utf-8")}')


if __name__ == "__main__":
    OfflineProcessorGui()
