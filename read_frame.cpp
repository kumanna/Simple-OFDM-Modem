#include <iostream>
#include "parameters.hpp"
#include "transmit.hpp"
#include "receive.hpp"

#define FREQ_OFFSET_ON true

int
main(int argc, char *argv[])
{
  RNG_randomize();

  // Get command-line parameters
  double snr_dB = (argc > 1) ? atof(argv[1]) : 3; // Get SNR from command line
  int bonus = (argc > 2) ? atoi(argv[2]) : 0;
  bool use_ldpc = (argc > 3) ? (argv[3][0] == '1') : false;
  const char *ldpc_code = (argc > 4) ? argv[4] : "mackay_204.33.484_opt.it";
  bool irregular = ldpc_code[0] == 'R';
  double snr = inv_dB(snr_dB);

  bvec bits;
  bvec recv_bits, encoded_bits;

  QAM qam(4);

  OFDM ofdm(NFFT, NCP);
  cvec modulated_symbols, received_symbols, transmitted_symbols, received_symbols_equalized, received_symbols_full;

  cvec estimation_sequence_symbol_bpsk = generate_special_estimation_sequence();
  cvec estimation_sequence_symbol = ofdm.modulate(estimation_sequence_symbol_bpsk);
  cvec channel_estimate_subcarriers;

  BERC berc;
  BERC berc_individual;
  //  BLERC blerc;

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
  int packet_length = (ceil(double(NBITS) / double(SYMBOLS_PER_ODFM) / 2.0) + 3) * (NFFT + NCP) + 48 + 160+16;

  cvec pilots, symbols, symbols_n, subvector;
  vec softbits;
  QLLRvec llr;
  symbols = "";

  it_file ff;
  ff.open("receive-data.it");
  ff >> Name("data") >> received_symbols_full;
  ff.close();
  ff.open("bits.it");
  ff >> Name("bits") >> bits;
  ff.close();

#define SCHMIDL_COX_LENGTH (48 + 176 + (NFFT + NCP) * 14 + 100)

  // Receive side
  int ldpc_n = C.get_nvar();
  int ldpc_k = C.get_nvar() - C.get_ncheck();
  while (received_symbols_full.length() > SCHMIDL_COX_LENGTH) {
    spc_timing_freq_recovery_wrap(received_symbols_full.left(SCHMIDL_COX_LENGTH), SCHMIDL_COX_LENGTH, PREAMBLE_LEN, NREPS_PREAMBLE, 0.1, &pos, &cfo_hat,  &pd);
    if (pd) { // If packet detected
      received_symbols = received_symbols_full.mid(pos - 1, SCHMIDL_COX_LENGTH);
      received_symbols_full.del(0, pos + SCHMIDL_COX_LENGTH - 1);
      received_symbols.del(0, NREPS_PREAMBLE * PREAMBLE_LEN - 1);
      received_symbols = received_symbols.left(packet_length - NREPS_PREAMBLE * PREAMBLE_LEN);

      // Giannakis frequency offset estimation
      cfo_hat_giannakis = estimate_frequency_offset(received_symbols.mid(16, 144), 1024);
#if FREQ_OFFSET_ON == true
      introduce_frequency_offset(received_symbols, - 2 * M_PI * cfo_hat_giannakis);
#endif
      received_symbols.del(0, 159+16);

      // Apply bonus (only new thing in the whole experiment)
      apply_bonus(received_symbols, bonus);

      // Frequency offset jugglery
      coarse_f = double(channel_coarse_frequency_estimate(ofdm, received_symbols.left(NREP_ESTIMATION_SYMBOL * (NFFT + NCP)), estimation_sequence_symbol_bpsk, channel_estimate_subcarriers));
#if FREQ_OFFSET_ON == true
      introduce_frequency_offset(received_symbols,-2*M_PI* coarse_f/NFFT);
#endif
      received_symbols.del(0, NREP_ESTIMATION_SYMBOL * (NFFT + NCP) - 1);
      channel_equalize_and_demodulate(ofdm, channel_estimate_subcarriers, received_symbols, received_symbols_equalized);

      symbols = "";
      for (int n = 0; n < received_symbols_equalized.length() / NFFT; ++n) {
	extract_ofdm_symbol(received_symbols_equalized.mid(n * NFFT, NFFT), pilots, symbols_n);
	symbols = concat(symbols, symbols_n);
      }
      symbols = fourth_power_derotate(symbols);
      recv_bits = "";
      if (!use_ldpc) {
	recv_bits = qam.demodulate_bits(symbols);
      }
      else {
	softbits = qam.demodulate_soft_bits(symbols, 1.0 / snr / 2.0 / C.get_rate(), LOGMAP);
	for (int k = 0; k < int(softbits.length() / ldpc_n); ++k) {
	  C.bp_decode(C.get_llrcalc().to_qllr(softbits.mid(k * ldpc_n, ldpc_n)), llr);
	  recv_bits = concat(recv_bits, (llr < 0).left(ldpc_k));
	}
      }
      berc.count(bits, recv_bits.left(bits.length()));
      berc_individual.clear();
      berc_individual.count(bits, recv_bits.left(bits.length()));
      // blerc.count(bits, recv_bits.left(bits.length()));
      n_successful_detects++;
      cout << snr_dB << "\t" << berc_individual.get_errorrate() << "\t" << n_successful_detects << endl;
    }
    else {
      received_symbols_full.del(0, 1);
    }
  }
  cout << snr_dB << "\t" << berc.get_errorrate() << "\t" << n_successful_detects << endl;
  //  cout << snr_dB << "\t" << berc.get_errorrate() << "\t" << blerc.get_errorrate() << "\t" << n_successful_detects << "\t" << iter << endl;
  return 0;
}
