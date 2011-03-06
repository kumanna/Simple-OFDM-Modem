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
  bool use_ldpc = (argc > 2) ? (argv[2][0] == '1') : false;
  const char *ldpc_code = (argc > 3) ? argv[3] : "mackay_204.33.484_opt.it";
  bool irregular = ldpc_code[0] == 'R';
  double snr = inv_dB(snr_dB);

  bvec bits;
  bvec recv_bits, encoded_bits;

  QAM qam(4);

  OFDM ofdm(NFFT, NCP);
  cvec modulated_symbols, received_symbols, transmitted_symbols, received_symbols_equalized;

  cvec estimation_sequence_symbol_bpsk = generate_special_estimation_sequence();
  cvec estimation_sequence_symbol = ofdm.modulate(estimation_sequence_symbol_bpsk);
  cvec channel_estimate_subcarriers;

  BERC berc;
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
  int packet_length = (ceil(double(NBITS) / double(SYMBOLS_PER_ODFM) / 2.0) + 3) * 69 + 48 + 160;

  cvec pilots, symbols, symbols_n, subvector;
  vec softbits;
  QLLRvec llr;
  symbols = "";

  it_file ff;
  ff.open("receive-data.it");
  ff >> Name("data") >> received_symbols;
  ff.close();
  ff.open("bits.it");
  ff >> Name("bits") >> bits;
  ff.close();

  // Receive side
  spc_timing_freq_recovery_wrap(received_symbols, received_symbols.length(), PREAMBLE_LEN, NREPS_PREAMBLE, 0.1, &pos, &cfo_hat,  &pd);
  if (pd) { // If packet detected
    //    received_symbols = received_symbols.left(packet_length);
    received_symbols.del(0, pos - 2 + NREPS_PREAMBLE * PREAMBLE_LEN);
    received_symbols = received_symbols.left(packet_length - NREPS_PREAMBLE * PREAMBLE_LEN);

    // Giannakis frequency offset estimation
    cfo_hat_giannakis = estimate_frequency_offset(received_symbols.mid(16, 144), 1024);
#if FREQ_OFFSET_ON == true
    introduce_frequency_offset(received_symbols, - 2 * M_PI * cfo_hat_giannakis);
#endif
    received_symbols.del(0, 159);

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
    symbols = fourth_power_derotate(symbols);
    if (!use_ldpc) {
      recv_bits = qam.demodulate_bits(symbols);
    }
    else {
      softbits = qam.demodulate_soft_bits(symbols, 1.0 / snr / 2.0 / C.get_rate(), LOGMAP);
      C.bp_decode(C.get_llrcalc().to_qllr(softbits.left(encoded_bits.length())), llr);
      recv_bits = llr < 0;
    }
    berc.count(bits, recv_bits.left(bits.length()));
    // blerc.count(bits, recv_bits.left(bits.length()));
  }
  cout << snr_dB << "\t" << berc.get_errorrate() << "\t" << endl;
  //  cout << snr_dB << "\t" << berc.get_errorrate() << "\t" << blerc.get_errorrate() << "\t" << n_successful_detects << "\t" << iter << endl;
  return 0;
}
