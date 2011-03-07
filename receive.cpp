#include "parameters.hpp"
#include "receive.hpp"
#include "unwrap.hpp"

void
channel_equalize_and_demodulate(OFDM &ofdm, const cvec &channel_estimate_subcarriers, const cvec &received_symbols, cvec &received_symbols_equalized)
{
  cvec received_symbols_subcarriers, received_symbols_subcarriers_n;
  received_symbols_equalized = "";
  for (int i = 0; i < received_symbols.length() / (NFFT + NCP); ++i) {
    received_symbols_subcarriers = ofdm.demodulate(received_symbols.mid(i * (NFFT + NCP), NFFT + NCP));
    received_symbols_subcarriers_n = elem_div(received_symbols_subcarriers, channel_estimate_subcarriers);
    received_symbols_equalized = concat(received_symbols_equalized, received_symbols_subcarriers_n);
  }
}

#define MAX_ROTATIONS 6

int
coarse_frequency_offset_estimate(const cvec &single_ofdm_symbol, const cvec &pilots_freq_ref)
{
  cvec rotated_pilots;
  double metric_max = -1.0, metric;
  double energy = 1.0;
  int max_offset;

  ivec correlation_locations = "1:63";

  ivec offset_vec(pilots_freq_ref.length());
  for (int i = -MAX_ROTATIONS; i < MAX_ROTATIONS + 1; ++i) {
    offset_vec = correlation_locations + i;
    for (int j = 0; j < pilots_freq_ref.length(); ++j) {
      offset_vec[j] = (offset_vec[j] < 0) ? offset_vec[j] + NFFT : ((offset_vec[j] > NFFT) ? offset_vec[j] - NFFT : offset_vec[j]);
    }
    rotated_pilots = single_ofdm_symbol(offset_vec);
    energy = sum(sqr(rotated_pilots));
    metric = abs(sum(elem_mult(rotated_pilots, conj(pilots_freq_ref(correlation_locations))))) / sqrt(energy);
    if (metric > metric_max) {
      max_offset = i;
      metric_max = metric;
    }
  }
  return max_offset;
}

int
channel_coarse_frequency_estimate(OFDM &ofdm, const cvec &pilots_time, const cvec &pilots_freq_ref, cvec &estimate)
{
  cvec pilots_freq = ofdm.demodulate(pilots_time);
  estimate = zeros_c(NFFT);

  for (int i = 0; i < pilots_freq.length() / NFFT; ++i) {
    estimate = estimate + elem_div(pilots_freq.mid(i * NFFT, NFFT), pilots_freq_ref);
  }

  estimate = estimate / NREP_ESTIMATION_SYMBOL;
  return coarse_frequency_offset_estimate(pilots_freq.left(NFFT), pilots_freq_ref);
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
  //  double phase = imag(log(cross_corr[sync_point]));
  double phase = arg(cross_corr[sync_point]);
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

void
estimate_ofdm_symbol(cvec &ofdm_subcarriers)
{
  cvec pilot_val = zeros_c(64);
  for (int i = -7; i < 7; ++i) {
    pilot_val[(i + 64) % 64] = ofdm_subcarriers[57] * double(7 - i) / 14.0 + ofdm_subcarriers[7] * double(14 - 7 + i) / 14.0;
  }
  for (int i = 7; i < 21; ++i) {
    pilot_val[i] = ofdm_subcarriers[7] * double(7 - i) / 14.0 + ofdm_subcarriers[21] * double(14 - 7 + i) / 14.0;
  }
  for (int i = 21; i < 43; ++i) {
    pilot_val[i] = ofdm_subcarriers[21] * double(7 - i) / 14.0 + ofdm_subcarriers[43] * double(14 - 7 + i) / 14.0;
  }
  for (int i = 43; i < 57; ++i) {
    pilot_val[i] = ofdm_subcarriers[43] * double(7 - i) / 14.0 + ofdm_subcarriers[57] * double(14 - 7 + i) / 14.0;
  }
  ofdm_subcarriers = elem_div(ofdm_subcarriers, pilot_val / complex<double>(1,1) * sqrt(2) + ALPHA);
}

void
extract_ofdm_symbol(const cvec &ofdm_symbol_subcarriers, cvec &pilots, cvec &symbols, bool awgn)
{
  pilots = zeros_c(4);
  int pilot_index = 0;
  symbols = zeros_c(SYMBOLS_PER_ODFM);
  int symbol_index = 0;
  cvec ofdm_symbol_subcarriers_modified = ofdm_symbol_subcarriers;
  //  if (!awgn) { estimate_ofdm_symbol(ofdm_symbol_subcarriers_modified); }
  for (int i = 0; i < NFFT; ++i) {
    switch(mask[i]) {
    case PILOT_SUBC:
      pilots[pilot_index++] = ofdm_symbol_subcarriers_modified[i];
      break;
    case DATA_SUBC:
      symbols[symbol_index++] = ofdm_symbol_subcarriers_modified[i];
      break;
    default:
      break;
    }
  }
}

#define Ns 16

double
estimate_frequency_offset(const cvec &c, int n_fft)
{
  vec fft_vals = zeros(n_fft);
  int index;
  double v = 0;
  cvec ys = zeros_c(Ns);
  for (int n = 0; n < Ns; ++n) {
    for (int j = 0; j < 9; ++j) {
      ys[j] = c(j * 16 + n);
    }
    fft_vals = fft_vals + sqr(fft(ys, n_fft));
  }
  max(fft_vals, index);
  index = (index > n_fft / 2) ? index - n_fft : index;
  v = double(index) / 16.0 / double(n_fft);
  return v;
}

cvec
fourth_power_derotate(const cvec &symbols)
{
  cvec fixed_vector = "";
  cvec subvector;
  cvec fourth_pow;
  vec angles;
  double average_phi;
  for (int i = 0; i < symbols.length() / FOURTH_POWER_WINDOW; ++i) {
    subvector = symbols.mid(FOURTH_POWER_WINDOW * i, FOURTH_POWER_WINDOW);
    fourth_pow = elem_mult(subvector, subvector);
    fourth_pow = -elem_mult(fourth_pow, fourth_pow);
    angles = arg(fourth_pow);
    //    unwrap(arg(fourth_pow), angles);
    average_phi = sum(angles) / FOURTH_POWER_WINDOW / 4.0;
    fixed_vector = concat(fixed_vector, subvector * exp(complex<double>(0,-1.0) * average_phi));
  }
  return fixed_vector;
}

void
apply_bonus(cvec &received_syms, int bonus)
{
  if (NCP < bonus) {
    cerr << "Bonus must be within cyclic prefix" << endl;
  }
  for (int i = 0; i < received_syms.length(); i = i + NFFT + NCP) {
    for (int j = 0; j < bonus; ++j) {
      received_syms[NCP - j + i] = 0.5 * (received_syms[NCP - j + i ] + received_syms[NCP - j + i + NFFT]);
    }
  }
}
