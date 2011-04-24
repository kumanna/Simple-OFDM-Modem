/**********************************************************************************/
/*      Copyright © 2011, Kumar Appaiah, Radha Krishna Ganti, Kannan Srinivasan  */
/*      The University of Texas at Austin                                         */
/*                                                                                */
/*      This file is part of Simple OFDM Modem.                                   */
/*                                                                                */
/*      Simple OFDM Modem is free software: you can redistribute it and/or        */
/*      modify it under the terms of the GNU General Public License as            */
/*      published by the Free Software Foundation, either version 3 of the        */
/*      License, or (at your option) any later version.                           */
/*                                                                                */
/*      Simple OFDM Modem is distributed in the hope that it will be              */
/*      useful, but WITHOUT ANY WARRANTY; without even the implied                */
/*      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.          */
/*      See the GNU General Public License for more details.                      */
/*                                                                                */
/*      You should have received a copy of the GNU General Public License         */
/*      along with Simple OFDM Modem.  If not, see                                */
/*      <http://www.gnu.org/licenses/>.                                           */
/*                                                                                */
/**********************************************************************************/

#include <iostream>
#include "parameters.hpp"
#include "receive.hpp"
#include "transmit.hpp"

#define NBITS 1024
#define ITER 1000
#define FREQ_OFFSET_ON true

int
main(int argc, char *argv[])
{
  RNG_randomize();

  // Get command-line parameters
  double snr_dB = (argc > 1) ? atof(argv[1]) : 3; // Get SNR from command line
  int iter = (argc > 2) ? atoi(argv[2]) : ITER;
  bool use_ldpc = (argc > 3) ? (argv[3][0] == '1') : false;
  const char *ldpc_code = (argc > 4) ? argv[4] : "mackay_204.33.484_opt.it";
  bool irregular = ldpc_code[0] == 'R';
  double snr = inv_dB(snr_dB);

  bvec bits;
  bvec recv_bits, encoded_bits;

  QAM qam(4);

  AWGN_Channel awgn_channel(1.0 / snr / 2.0);
  OFDM ofdm(NFFT, NCP);
  cvec modulated_symbols, received_symbols, transmitted_symbols, received_symbols_equalized;

  cvec estimation_sequence_symbol_bpsk = generate_special_estimation_sequence();
  cvec estimation_sequence_symbol = ofdm.modulate(estimation_sequence_symbol_bpsk);
  cvec channel_estimate_subcarriers;

  BERC berc;
  BLERC blerc;
  bool blocksize_set = false;

  // Transmit side
  LDPC_Generator_Systematic G; // for codes created with ldpc_gen_codes since generator exists
  LDPC_Code C(ldpc_code, &G);
  C.set_exit_conditions(250);
  //  C.set_llrcalc(LLR_calc_unit(12,0,7));
  //  if (use_ldpc) { cout << C << endl;  }

  int pos; // Frame start marker
  int pd; // Detected packet: yes/no
  double cfo_hat, cfo_hat_giannakis; // frequency offset
  int coarse_f = 0;
  int n_successful_detects = 0;
  int packet_length = 0;

  cvec pilots, symbols, symbols_n, subvector;
  vec softbits;
  QLLRvec llr;
  for (int i = 0; i < iter; ++i) {
    symbols = "";
    // Transmit side
    encoded_bits = zeros_b(C.get_nvar());
    if (!irregular) {
      bits = use_ldpc ? randb(C.get_nvar() - C.get_ncheck()) : randb(NBITS);
      encoded_bits = use_ldpc ? C.encode(bits) : bits;
    }
    else if (use_ldpc) {
      bits = zeros_b(C.get_nvar() - C.get_ncheck());
    }
    if (!blocksize_set) {
      blerc.set_blocksize(bits.size());
      blocksize_set = true;
    }

    fill_bits_into_ofdm_symbols(encoded_bits, qam, ofdm, estimation_sequence_symbol, &packet_length, modulated_symbols);

    // Channel
    transmitted_symbols = concat(zeros_c(100), modulated_symbols, zeros_c(10));
    received_symbols = awgn_channel(transmitted_symbols);
#if FREQ_OFFSET_ON == true
    introduce_frequency_offset(received_symbols, 0.02);
#endif

    // Receive side
    spc_timing_freq_recovery_wrap(received_symbols, received_symbols.length(), PREAMBLE_LEN, NREPS_PREAMBLE, 0.1, &pos, &cfo_hat,  &pd);
    if (pd) { // If packet detected
      received_symbols.del(0, pos - 2 + NREPS_PREAMBLE * PREAMBLE_LEN);
      received_symbols = received_symbols.left(packet_length);

      // Giannakis frequency offset estimation
      cfo_hat_giannakis = estimate_frequency_offset(received_symbols.mid(16, 144), 1024);
#if FREQ_OFFSET_ON == true
      introduce_frequency_offset(received_symbols, - 2 * M_PI * cfo_hat_giannakis);
#endif
      received_symbols.del(0, 159+16);

      // Frequency offset jugglery
      coarse_f = double(channel_coarse_frequency_estimate(ofdm, received_symbols.left(NREP_ESTIMATION_SYMBOL * (NFFT + NCP)), estimation_sequence_symbol_bpsk, channel_estimate_subcarriers));
#if FREQ_OFFSET_ON == true
      introduce_frequency_offset(received_symbols,-2*M_PI* coarse_f/NFFT);
#endif
      received_symbols.del(0, NREP_ESTIMATION_SYMBOL * (NFFT + NCP) - 1);
      channel_equalize_and_demodulate(ofdm, channel_estimate_subcarriers, received_symbols, received_symbols_equalized);

      for (int n = 0; n < received_symbols_equalized.length() / NFFT; ++n) {
       extract_ofdm_symbol(received_symbols_equalized.mid(n * NFFT, NFFT), pilots, symbols_n);
       symbols = concat(symbols, symbols_n);
      }
      // cout << "x = " << symbols << ";" << endl;
      symbols = fourth_power_derotate(symbols);
      // cout << "y = " << symbols << ";" << endl;
      // exit(1);
      if (!use_ldpc) {
	recv_bits = qam.demodulate_bits(symbols);
      }
      else {
	softbits = qam.demodulate_soft_bits(symbols, 1.0 / snr / 2.0 / C.get_rate(), LOGMAP);
	C.bp_decode(C.get_llrcalc().to_qllr(softbits.left(encoded_bits.length())), llr);
	recv_bits = llr < 0;
      }
      berc.count(bits, recv_bits.left(bits.length()));
      blerc.count(bits, recv_bits.left(bits.length()));
    }
    n_successful_detects = n_successful_detects + pd;
  }
  cout << snr_dB << "\t" << berc.get_errorrate() << "\t" << blerc.get_errorrate() << "\t" << n_successful_detects << "\t" << iter << endl;
  return 0;
}
