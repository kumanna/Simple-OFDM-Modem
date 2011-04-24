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
