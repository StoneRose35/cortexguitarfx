from filter_calculations import *

if __name__ == "__main__":
    width = 700
    center = 1300
    sos = design_and_plot_iir_filter(True, rs=30, fc=[center-width,center+width], do_overflow=False,
                                     sample_size=15, type="butter", ftype="bandpass")
