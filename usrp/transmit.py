#!/usr/bin/env python
"""
test

"""

INTERP = 32
TXGAIN = 20
CONSTANT = 10.0

from gnuradio import gr, gr_unittest
import usrp_options
from optparse import OptionParser
from gnuradio.eng_option import eng_option
from pick_bitrate import pick_tx_bitrate

def main():
    gr.enable_realtime_scheduling()
    tb = gr.top_block ()
    src = gr.file_source(gr.sizeof_gr_complex, "transmit-data.dat", True)
    parser = OptionParser(option_class=eng_option, conflict_handler="resolve")
    (options, args) = parser.parse_args ()
    d = {'verbose': True, 'discontinuous': False, 'samples_per_symbol': 2, 'usrpx': None, 'interp': INTERP, 'fusb_block_size': 0, 'megabytes': 1.0, 'rx_freq': 2.44e9, 'size': 1500, 'show_tx_gain_range': False, 'log': False, 'tx_subdev_spec': None, 'fusb_nblocks': 0, 'lo_offset': None, 'tx_gain': TXGAIN, 'which': 0, 'modulation': 'gmsk', 'excess_bw': 0.34999999999999998, 'bt': 0.34999999999999998, 'interface': 'eth0', 'freq': None, 'bitrate': 100000.0, 'from_file': None, 'tx_freq': 2412000000.0, 'mac_addr': '', 'tx_amplitude': 0.1, 'gray_code': True}
    for i, j in d.items():
        setattr(options, i, j)

    u = usrp_options.create_usrp_sink(options)
    dac_rate = u.dac_rate()
    if options.verbose:
        print 'USRP Sink:', u
    (_bitrate, _samples_per_symbol, _interp) = \
                    pick_tx_bitrate(options.bitrate, 2, \
                                    options.samples_per_symbol, options.interp, dac_rate, \
                                    u.get_interp_rates())

    u.set_interp(_interp)
    u.set_auto_tr(True)

    if not u.set_center_freq(options.tx_freq):
        print "Failed to set Rx frequency to %s" % (eng_notation.num_to_str(options.tx_freq))
        raise ValueError, eng_notation.num_to_str(options.tx_freq)
    
    m = gr.multiply_const_cc(CONSTANT)
    tb.connect(src, m, u)


    tb.run()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print "Bye"
