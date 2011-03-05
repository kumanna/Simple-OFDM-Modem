#ifndef UNWRAP_HPP
#define UNWRAP_HPP

#include <itpp/itcomm.h>

using namespace itpp;

void
unwrap(const vec &phases,  vec &unwrapped, double tol = M_PI);

#endif
