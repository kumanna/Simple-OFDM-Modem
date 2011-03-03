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
  cvec modulated_symbols, received_symbols;

  BERC berc;
  for (int i = 0; i < iter; ++i) {
    bits = randb(NBITS);
    modulated_symbols = ofdm.modulate(qam.modulate_bits(bits));
    received_symbols = awgn_channel(modulated_symbols);
    recv_bits = qam.demodulate_bits(ofdm.demodulate(received_symbols));
    berc.count(bits, recv_bits);
  }
  cout << snr_dB << "\t" << berc.get_errorrate() << endl;
  return 0;
}
