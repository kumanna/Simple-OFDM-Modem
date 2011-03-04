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
  cvec ofdm_symbol_subcarriers = zeros_c(ofdm.no_carriers());
  cvec symbols = qam.modulate_bits(bits);
  for (int i = 0; i < ofdm.no_carriers(); ++i) {
    switch(symbol_mask[i]) {
    case ZERO_SUBC:
      ofdm_symbol_subcarriers[i] = 0;
      break;
    case PILOT_SUBC:
      // This is subject to change, since identical pilots might be a problem
      ofdm_symbol_subcarriers[i] = complex<double>(1,1) / sqrt(2);
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
  int bits_per_ofdm_symbol = qam.bits_per_symbol() * SYMBOLS_PER_ODFM;

  // Add some symbols for frequency offset compensation
  // Giannakis symbols

  // OFDM ofdm_small(16,0);
  // cvec short_symbol;
  // create_ofdm_symbol(short_ofdm_symbol_data, short_mask, qam, ofdm_small, short_symbol);
  // ofdm_syms = concat(repmat(short_symbol, 10), ofdm_syms);

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

  *packet_length = ofdm_syms.length();

  // Prepend preamble
  ofdm_syms = concat(repmat(PREAMBLE_TONES, NREPS_PREAMBLE / 2) * PREAMBLE_GAIN, ofdm_syms);
  return actual_bits.length();
}
