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

#include "unwrap.hpp"

void
unwrap(const vec &phases, vec &unwrapped, double tol) {
  int len = phases.length();

  // Don't let anyone use a negative value for TOL.
  tol = abs(tol);
  
  double rng = 2 * M_PI;

  // Take first order difference to see so that wraps will show up
  // as large values, and the sign will show direction.
  vec d = -concat(vec("0"), phases.right(len - 1) - phases.left(len - 1));

  // Find only the peaks, and multiply them by the range so that there
  // are kronecker deltas at each wrap point multiplied by the range
  // value.
  vec p = zeros(len);
  for (int i = 0; i < len; ++i) {
    if (d[i] > tol) {
      p[i] = rng;
    }
    else if (d[i] < -tol) {
      p[i] = -rng;
    }
  }
  
  // Now need to "integrate" this so that the deltas become steps.
  vec r = cumsum(p);

  // Now add the "steps" to the original data and put output in the
  // same shape as originally.
  unwrapped = phases + r;
}
