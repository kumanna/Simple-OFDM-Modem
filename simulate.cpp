#include <iostream>
#include "parameters.hpp"
#include "receive.hpp"

#define NBITS 1024
#define ITER 1000

int
main(int argc, char *argv[])
{
  RNG_randomize();

  // Get command-line parameters
  double snr_dB = (argc > 1) ? atof(argv[1]) : 3; // Get SNR from command line
  int iter = (argc > 2) ? atoi(argv[2]) : ITER;
  double snr = inv_dB(snr_dB);

  bvec bits;
  bvec recv_bits;

  QAM qam(4);

  AWGN_Channel awgn_channel(1.0 / snr / 2.0);
  OFDM ofdm(NFFT, NCP);
  cvec modulated_symbols, received_symbols, transmitted_symbols;

  BERC berc;

  int pos; // Frame start marker
  int pd; // Detected packet: yes/no
  double cfo_hat; // frequency offset
  int n_successful_detects = 0;
  int packet_length = 0;
  for (int i = 0; i < iter; ++i) {
    // Transmit side
    bits = randb(NBITS);
    modulated_symbols = ofdm.modulate(qam.modulate_bits(bits));
    transmitted_symbols = concat(repmat(PREAMBLE_TONES, NREPS_PREAMBLE / 2) * PREAMBLE_GAIN, modulated_symbols);
    packet_length = modulated_symbols.length();

    // Channel
    transmitted_symbols = concat(zeros_c(100), transmitted_symbols, zeros_c(randi(30, 50)));
    received_symbols = awgn_channel(transmitted_symbols);

    // Receive side
    spc_timing_freq_recovery_wrap(received_symbols, received_symbols.length(), PREAMBLE_LEN, NREPS_PREAMBLE, 0.1, &pos, &cfo_hat,  &pd);
    if (pd) {
      received_symbols.del(0, pos - 2 + NREPS_PREAMBLE * PREAMBLE_LEN);
      received_symbols = received_symbols.left(packet_length);
      recv_bits = qam.demodulate_bits(ofdm.demodulate(received_symbols));
      berc.count(bits, recv_bits);
    }
    n_successful_detects = n_successful_detects + pd;
  }
  cout << snr_dB << "\t" << berc.get_errorrate() << "\t" << n_successful_detects << "\t" << iter << endl;
  return 0;
}
