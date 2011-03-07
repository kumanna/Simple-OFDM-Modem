#ifndef RECEIVE_HPP
#define RECEIVE_HPP

#include <itpp/itcomm.h>
using namespace itpp;

void
spc_timing_freq_recovery_wrap(const cvec& databuffer, int l_databuffer, int l_preambletones, int nreps_preamble, double metric_tol, int *pos, double *cfo_hat, int *pd);

void
channel_estimate(OFDM &ofdm, const cvec &pilots_time, const cvec &pilots_freq_ref, cvec &estimate);

void
channel_equalize_and_demodulate(OFDM &ofdm, const cvec &channel_estimate_subcarriers, const cvec &received_symbols, cvec &received_symbols_equalized);

void
introduce_frequency_offset(cvec &c, double offset);

void
extract_ofdm_symbol(const cvec &ofdm_symbol_subcarriers, cvec &pilots, cvec &symbols, bool awgn = false);

int
channel_coarse_frequency_estimate(OFDM &ofdm, const cvec &pilots_time, const cvec &pilots_freq_ref, cvec &estimate);

double
estimate_frequency_offset(const cvec &c, int n_fft);

cvec
fourth_power_derotate(const cvec &);

void
apply_bonus(cvec &received_syms, int bonus);
#endif
