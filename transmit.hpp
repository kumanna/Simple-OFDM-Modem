#ifndef TRANSMIT_HPP
#define TRANSMIT_HPP

#include "parameters.hpp"

cvec
generate_special_estimation_sequence();

int
fill_bits_into_ofdm_symbols(const bvec &bits, QAM &qam, OFDM &ofdm, cvec &special_estimation_seq, int *packet_length, cvec &ofdm_syms);

#endif
