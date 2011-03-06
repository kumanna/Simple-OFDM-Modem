#include <iostream>
#include "parameters.hpp"
#include "receive.hpp"
#include "transmit.hpp"

#define ITER 1

int
main(int argc, char *argv[])
{
  RNG_randomize();

  // Get command-line parameters
  int iter = (argc > 1) ? atoi(argv[1]) : ITER;
  bool use_ldpc = (argc > 2) ? (argv[2][0] == '1') : false;
  const char *ldpc_code = (argc > 3) ? argv[3] : "mackay_204.33.484_opt.it";
  bool irregular = ldpc_code[0] == 'R';

  bvec bits;
  bvec recv_bits, encoded_bits;

  QAM qam(4);

  OFDM ofdm(NFFT, NCP);
  cvec modulated_symbols, received_symbols, transmitted_symbols, received_symbols_equalized;

  cvec estimation_sequence_symbol_bpsk = generate_special_estimation_sequence();
  cvec estimation_sequence_symbol = ofdm.modulate(estimation_sequence_symbol_bpsk);
  cvec channel_estimate_subcarriers;
  int packet_length;

  // Transmit side
  LDPC_Generator_Systematic G; // for codes created with ldpc_gen_codes since generator exists
  LDPC_Code C(ldpc_code, &G);
  C.set_exit_conditions(250);

  // Transmit side
  encoded_bits = zeros_b(C.get_nvar());
  if (!irregular) {
    bits = use_ldpc ? randb(C.get_nvar() - C.get_ncheck()) : randb(NBITS);
    encoded_bits = use_ldpc ? C.encode(bits) : bits;
  }
  else if (use_ldpc) {
    bits = zeros_b(C.get_nvar() - C.get_ncheck());
  }
  fill_bits_into_ofdm_symbols(encoded_bits, qam, ofdm, estimation_sequence_symbol, &packet_length, modulated_symbols);
  
  modulated_symbols = concat(modulated_symbols, zeros_c(200));
  it_file ff;
  ff.open("transmit-data.it");
  ff << Name("data") <<   repmat(modulated_symbols, iter);
  ff.flush();
  ff.close();
  ff.open("bits.it");
  ff << Name("bits") <<   repmat(bits, iter);
  ff.flush();
  ff.close();

  return 0;
}
