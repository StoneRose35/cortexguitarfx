from filter_calculations import *

if __name__ == "__main__":
    width = 400
    center = 600
    sos = design_and_plot_iir_filter(True, rs=3, fc=[center - width, center + width], do_overflow=False,
                                     sample_size=15, type="cheby1", ftype="bandstop")
