#include <iostream>
#include "parameters.hpp"
#include "receive.hpp"
#include "transmit.hpp"

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
  cvec modulated_symbols, received_symbols, transmitted_symbols, received_symbols_equalized;

  cvec estimation_sequence_symbol_bpsk = generate_special_estimation_sequence();
  cvec estimation_sequence_symbol = ofdm.modulate(estimation_sequence_symbol_bpsk);
  cvec channel_estimate_subcarriers;

  BERC berc;

  int pos; // Frame start marker
  int pd; // Detected packet: yes/no
  double cfo_hat, cfo_hat_initial; // frequency offset
  int coarse_f = 0;
  int n_successful_detects = 0;
  int packet_length = 0;

  cvec pilots, symbols, symbols_n, subvector;
  for (int i = 0; i < iter; ++i) {
    symbols = "";
    // Transmit side
    bits = randb(NBITS);
    fill_bits_into_ofdm_symbols(bits, qam, ofdm, estimation_sequence_symbol, &packet_length, modulated_symbols);

    // Channel
    transmitted_symbols = concat(zeros_c(100), modulated_symbols, zeros_c(10));
    received_symbols = awgn_channel(transmitted_symbols);
    introduce_frequency_offset(received_symbols, 0.01);
//	cout<<"The introduced frequency offset"<< 2*M_PI/64*0.02<<endl;

    // Receive side
	// Timing Recovery
    spc_timing_freq_recovery_wrap(received_symbols, received_symbols.length(), PREAMBLE_LEN, NREPS_PREAMBLE, 0.1, &pos, &cfo_hat,  &pd);

 //cout<<"cfo hat_1: "<< cfo_hat<<endl;

 
 if (pd) { // If packet detected
      received_symbols.del(0, pos - 2 + NREPS_PREAMBLE * PREAMBLE_LEN);
      received_symbols = received_symbols.left(packet_length);

      // Frequency offset jugglery
      subvector = received_symbols.mid(30, 3 * (NFFT + NCP));
      spc_timing_freq_recovery_wrap(subvector, subvector.length(), (NFFT + NCP), 2, 0.1, &pos, &cfo_hat,  &pd);
//     cout<<"second Freq offset"<<"\t" << cfo_hat<<endl;
         introduce_frequency_offset(received_symbols, - cfo_hat);

      coarse_f = double(channel_coarse_frequency_estimate(ofdm, received_symbols.left(NREP_ESTIMATION_SYMBOL * (NFFT + NCP)), estimation_sequence_symbol_bpsk, channel_estimate_subcarriers));
   introduce_frequency_offset(received_symbols,-2*M_PI* coarse_f/(NFFT+NCP));
//cout<< received_symbols;
	  cout<< "total_estimate"<<"\t"<<2*M_PI* coarse_f/(NFFT+NCP)+ cfo_hat<<endl;


      received_symbols.del(0, NREP_ESTIMATION_SYMBOL * (NFFT + NCP) - 1);
      channel_equalize_and_demodulate(ofdm, channel_estimate_subcarriers, received_symbols, received_symbols_equalized);

      for (int n = 0; n < received_symbols_equalized.length() / NFFT; ++n) {
       extract_ofdm_symbol(received_symbols_equalized.mid(n * NFFT, NFFT), pilots, symbols_n);
	   symbols = concat(symbols, symbols_n);
      }
      recv_bits = qam.demodulate_bits(symbols);
      berc.count(bits, recv_bits.left(bits.length()));
    }
    n_successful_detects = n_successful_detects + pd;
  }
  cout << snr_dB << "\t" << berc.get_errorrate() << "\t" << n_successful_detects << "\t" << iter << endl;
  return 0;
}
