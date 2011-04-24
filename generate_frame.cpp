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
  int n = C.get_nvar();
  int k = C.get_nvar() - C.get_ncheck();
  encoded_bits = "";
  if (!irregular) {
    bits = use_ldpc ? randb((C.get_nvar() - C.get_ncheck()) * int(NBITS / n)) : randb(NBITS);
    if (use_ldpc) {
      for (int m = 0; m < int(NBITS / n); ++m) {
	encoded_bits = concat(encoded_bits, C.encode(bits.mid(m * k, k)));
      }
    }
    else {
      encoded_bits = bits;
    }
  }
  else if (use_ldpc) {
    bits = zeros_b(C.get_nvar() - C.get_ncheck());
  }
  fill_bits_into_ofdm_symbols(encoded_bits, qam, ofdm, estimation_sequence_symbol, &packet_length, modulated_symbols);
  
  cout << "Modulated length (packet): " << modulated_symbols.length() << endl;
  cout << "Data bits: " << bits.length() << endl;
  cout << "Encoded bits: " << encoded_bits.length() << endl;
  modulated_symbols = concat(modulated_symbols, zeros_c(200));
  it_file ff;
  ff.open("transmit-data.it");
  ff << Name("data") <<   repmat(modulated_symbols, iter);
  ff.flush();
  ff.close();
  ff.open("bits.it");
  ff << Name("bits") << bits;
  ff.flush();
  ff.close();

  return 0;
}
