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
#include "transmit.hpp"

cvec
generate_special_estimation_sequence()
{
  BPSK_c bpsk;
  // Check the power of this OFDM symbol
  return bpsk.modulate_bits(special_estimation_seq);
}

void
create_ofdm_symbol(const bvec &bits, const int *symbol_mask, QAM &qam, OFDM &ofdm, cvec &ofdm_symbol)
{
  int symbol_index = 0;
  int pilot_index = 0;
  cvec ofdm_symbol_subcarriers = zeros_c(ofdm.no_carriers());
  cvec symbols = qam.modulate_bits(bits);
  for (int i = 0; i < ofdm.no_carriers(); ++i) {
    switch(symbol_mask[i]) {
    case ZERO_SUBC:
      ofdm_symbol_subcarriers[i] = 0;
      break;
    case PILOT_SUBC:
      // This is subject to change, since identical pilots might be a problem
      ofdm_symbol_subcarriers[i] = PILOT_VALUES[pilot_index++];
      break;
    case DATA_SUBC:
      ofdm_symbol_subcarriers[i] = symbols[symbol_index++];
      break;
    default:
      break;
    }
  }

  // The extra scaling is to undo the scaling IT++ applies to OFDM
  // symbols for it's normalization after addition of cyclic prefix,
  // and account for zero subcarriers
  ofdm_symbol = ofdm.modulate(ofdm_symbol_subcarriers);// * sqrt(1.0 * (NFFT + NCP) / NFFT * 64.0 / 52.0);
}

int
fill_bits_into_ofdm_symbols(const bvec &bits, QAM &qam, OFDM &ofdm, cvec &special_estimation_seq, int *packet_length, cvec &ofdm_syms)
{
  ofdm_syms = "";
  cvec freq_offset_symbols = "";
  int bits_per_ofdm_symbol = qam.bits_per_symbol() * SYMBOLS_PER_ODFM;

  // Add some symbols for frequency offset compensation
  // Giannakis symbols

  OFDM ofdm_small(16,0);
  cvec short_symbol;
  create_ofdm_symbol(short_ofdm_symbol_data, short_mask, qam, ofdm_small, short_symbol);
  freq_offset_symbols = concat(repmat(short_symbol, 11), freq_offset_symbols);

  bvec actual_bits = bits; // The actual padded bit sequence

  int padding_remainder = actual_bits.length() % bits_per_ofdm_symbol;
  if (padding_remainder != 0) {
    actual_bits = concat(actual_bits, randb(bits_per_ofdm_symbol - padding_remainder));
  }
  int n_ofdm_syms = actual_bits.length() / bits_per_ofdm_symbol;
  for (int i = 0; i < n_ofdm_syms; ++i) {
    cvec ofdm_symbol_n;
    create_ofdm_symbol(actual_bits.mid(i * bits_per_ofdm_symbol, bits_per_ofdm_symbol), mask, qam, ofdm, ofdm_symbol_n);
    ofdm_syms = concat(ofdm_syms, ofdm_symbol_n);
  }

  // Prepend a special symbol to aid estimation
  ofdm_syms = concat(repmat(special_estimation_seq, NREP_ESTIMATION_SYMBOL), ofdm_syms);

  // Prepend frequency offset symbols
  ofdm_syms = concat(freq_offset_symbols, ofdm_syms);

  *packet_length = ofdm_syms.length();

  // Prepend preamble
  ofdm_syms = concat(repmat(PREAMBLE_TONES, NREPS_PREAMBLE / 2) * PREAMBLE_GAIN, ofdm_syms);
  return actual_bits.length();
}
