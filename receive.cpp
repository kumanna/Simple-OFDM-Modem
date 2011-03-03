#include "parameters.hpp"
#include "receive.hpp"

void
channel_equalize(OFDM &ofdm, const cvec &channel_estimate_subcarriers, const cvec &received_symbols, cvec &received_symbols_equalized)
{
  cvec received_symbols_subcarriers, received_symbols_subcarriers_n;
  received_symbols_equalized = "";
  for (int i = 0; i < received_symbols.length() / (NFFT + NCP); ++i) {
    received_symbols_subcarriers = ofdm.demodulate(received_symbols.mid(i * (NFFT + NCP), NFFT + NCP));
    received_symbols_subcarriers_n = elem_div(received_symbols_subcarriers, channel_estimate_subcarriers);
    received_symbols_equalized = concat(received_symbols_equalized, received_symbols_subcarriers_n);
  }
}

void
channel_estimate(OFDM &ofdm, const cvec &pilots_time, const cvec &pilots_freq_ref, cvec &estimate)
{
  cvec pilots_freq = ofdm.demodulate(pilots_time);
  estimate = zeros_c(NFFT);

  for (int i = 0; i < pilots_freq.length() / NFFT; ++i) {
    estimate = estimate + elem_div(pilots_freq.mid(i * NFFT, NFFT), pilots_freq_ref);
  }
  estimate = estimate / NREP_ESTIMATION_SYMBOL;
}
void
spc_timing_freq_recovery_wrap(const cvec& databuffer, int l_databuffer, int l_preambletones, int nreps_preamble, double metric_tol, int *pos, double *cfo_hat, int *pd)
{  
  int corrlength = l_preambletones;
  vec auto_corr(l_databuffer - 2 * corrlength + 1);
  int i;
  int k;
  auto_corr[0] = 0;
  cvec cross_corr(l_databuffer - (2 * corrlength) + 1);
  vec metric(l_databuffer - (2 * corrlength) + 1);
  *pos = -1;
  *pd=0;
  for (i = 0; i < corrlength; i++) {
    cross_corr[0] = cross_corr[0] + conj(databuffer[i]) * databuffer[i + corrlength];
    auto_corr[0] += 0.5*(sqr(databuffer[i]) + sqr(databuffer[i+corrlength]));
  }
  metric[0] = abs(cross_corr[0]) / auto_corr[0];
  complex<double> temp1;
  complex<double> temp2;
  for ( k = 1; k < l_databuffer - 2 * corrlength + 1; k++) {
    temp1 = conj(databuffer[k-1])* databuffer[k+corrlength-1];
    temp2 = conj(databuffer[k+corrlength-1])*databuffer[k+2*corrlength-1];
    cross_corr[k] = cross_corr[k-1]- temp1 + temp2;

    auto_corr[k] = auto_corr[k - 1] - 0.5 * sqr(databuffer[k - 1])
      + 0.5 * sqr(databuffer[k + 2* corrlength - 1]);
        
    metric[k] = abs(cross_corr[k]) / abs(auto_corr[k]);
  }

  int sync_point = 0;
  float sync_value = -std::numeric_limits<float>::infinity();
    
  float tmp;
  for (i = 0; i < l_databuffer - (2 * corrlength); i++) {
    tmp = abs(cross_corr[i]);
    if (sync_value < tmp) {
      sync_value = tmp;
      sync_point = i;
    }
  }
  // printf(" The sync point is %d \n", sync_point);
  double phase = imag(log(cross_corr[sync_point]));
  float absmetric =abs(metric[sync_point]);

  if ((absmetric <= (1 + metric_tol)) && (absmetric >= (1 - metric_tol))
      && (sync_point != (l_databuffer - 2* corrlength - 1))) {

    *pos = sync_point + 1;
    phase = phase > 0 ? fmod(phase, M_PI) : fmod(phase, -M_PI);
    *cfo_hat = phase / corrlength; 
    *pd=1;
  }
}

void
introduce_frequency_offset(cvec &c, double offset)
{
  for (int i = 0; i < c.length(); ++i) {
    c[i] = c[i] * std::exp(complex<double>(0, 1) * offset * double(i));
  }
}
