#!/usr/bin/env python
"""
test

"""
RXGAIN = 45
DECIM = 64

import math
from gnuradio import gr, gr_unittest
import usrp_options
from optparse import OptionParser
from gnuradio.eng_option import eng_option
from pick_bitrate import pick_rx_bitrate, pick_tx_bitrate

def main():
    gr.enable_realtime_scheduling()
    parser = OptionParser(option_class=eng_option, conflict_handler="resolve")
    (options, args) = parser.parse_args ()
    d = {'verbose': True, 'samples_per_symbol': 2, 'usrpx': None, 'excess_bw': 0.34999999999999998, 'fusb_block_size': 0, 'log': False, 'costas_alpha': 0.14999999999999999, 'bitrate': 100000.0, 'decim': DECIM, 'omega_relative_limit': 0.0050000000000000001, 'fusb_nblocks': 0, 'which': 0, 'rx_subdev_spec': None, 'freq_error': 0.0, 'lo_offset': None, 'modulation': 'gmsk', 'gain_mu': None, 'interface': 'eth0', 'freq': None, 'rx_freq': 2.475e9, 'rx_gain': RXGAIN, 'tx_freq': 2440000000.0, 'mu': 0.5, 'mac_addr': '', 'show_rx_gain_range': True, 'gray_code': True, 'tx_subdev_spec' : None, 'tx_gain' : None, 'show_tx_gain_range': False}
    for i, j in d.items():
        setattr(options, i, j)

    u = usrp_options.create_usrp_source(options)
    adc_rate = u.adc_rate()
    if options.verbose:
        print 'USRP Source:', u
    (_bitrate, _samples_per_symbol, _decim) = \
                    pick_rx_bitrate(options.bitrate, 2, \
                                    options.samples_per_symbol, options.decim, adc_rate,  \
                                    u.get_decim_rates())

    u.set_decim(_decim)

    if not u.set_center_freq(options.rx_freq):
        print "Failed to set Rx frequency to %s" % (eng_notation.num_to_str(options.rx_freq))
        raise ValueError, eng_notation.num_to_str(options.rx_freq)

    tb = gr.top_block()
    dst = gr.vector_sink_s ()
    dump = gr.file_sink(gr.sizeof_gr_complex, 'packet.dat')
    tb.connect(u, dump)
    tb.run()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print "Bye"
