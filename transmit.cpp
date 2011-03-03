#include <iostream>
#include "transmit.hpp"

cvec
generate_special_estimation_sequence()
{
  BPSK_c bpsk;
  // Check the power of this OFDM symbol
  return bpsk.modulate_bits(special_estimation_seq);
}
