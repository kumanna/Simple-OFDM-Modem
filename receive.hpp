#ifndef RECEIVE_HPP
#define RECEIVE_HPP

#include <itpp/itcomm.h>
using namespace itpp;

void
spc_timing_freq_recovery_wrap(const cvec& databuffer, int l_databuffer, int l_preambletones, int nreps_preamble, double metric_tol, int *pos, double *cfo_hat, int *pd);

#endif
