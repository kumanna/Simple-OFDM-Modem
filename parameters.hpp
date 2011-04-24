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

#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <itpp/itcomm.h>

using namespace itpp;
using namespace std;

#define NFFT 64
#define NCP 63
#define SYMBOLS_PER_ODFM 48

#define PREAMBLE_LEN 24
#define NREPS_PREAMBLE 2
#define PREAMBLE_GAIN 2

#define NREP_ESTIMATION_SYMBOL 3

#define ALPHA 1e-2

enum SUBCARRIER_TYPE {ZERO_SUBC, DATA_SUBC, PILOT_SUBC};

const int mask[] = {ZERO_SUBC, // 0
		    DATA_SUBC, // 1
		    DATA_SUBC, // 2
		    DATA_SUBC, // 3
		    DATA_SUBC, // 4
		    DATA_SUBC, // 5
		    DATA_SUBC, // 6
		    PILOT_SUBC, // 7
		    DATA_SUBC, // 8
		    DATA_SUBC, // 9
		    DATA_SUBC, // 10
		    DATA_SUBC, // 11
		    DATA_SUBC, // 12
		    DATA_SUBC, // 13
		    DATA_SUBC, // 14
		    DATA_SUBC, // 15
		    DATA_SUBC, // 16
		    DATA_SUBC, // 17
		    DATA_SUBC, // 18
		    DATA_SUBC, // 19
		    DATA_SUBC, // 20
		    PILOT_SUBC, // 21
		    DATA_SUBC, // 22
		    DATA_SUBC, // 23
		    DATA_SUBC, // 24
		    DATA_SUBC, // 25
		    DATA_SUBC, // 26
		    ZERO_SUBC, // 27
		    ZERO_SUBC, // 28
		    ZERO_SUBC, // 29
		    ZERO_SUBC, // 30
		    ZERO_SUBC, // 31
		    ZERO_SUBC, // 32
		    ZERO_SUBC, // 33 == -31
		    ZERO_SUBC, // 34 == -30
		    ZERO_SUBC, // 35 == -29
		    ZERO_SUBC, // 36 == -28
		    ZERO_SUBC, // 37 == -27
		    DATA_SUBC, // 38 == -26
		    DATA_SUBC, // 39 == -25
		    DATA_SUBC, // 40 == -24
		    DATA_SUBC, // 41 == -23
		    DATA_SUBC, // 42 == -22
		    PILOT_SUBC, // 43 == -21
		    DATA_SUBC, // 44 == -20
		    DATA_SUBC, // 45 == -19
		    DATA_SUBC, // 46 == -18
		    DATA_SUBC, // 47 == -17
		    DATA_SUBC, // 48 == -16
		    DATA_SUBC, // 49 == -15
		    DATA_SUBC, // 50 == -14
		    DATA_SUBC, // 51 == -13
		    DATA_SUBC, // 52 == -12
		    DATA_SUBC, // 53 == -11
		    DATA_SUBC, // 54 == -10
		    DATA_SUBC, // 55 == -9
		    DATA_SUBC, // 56 == -8
		    PILOT_SUBC, // 57 == -7
		    DATA_SUBC, // 58 == -6
		    DATA_SUBC, // 59 == -5
		    DATA_SUBC, // 60 == -4
		    DATA_SUBC, // 61 == -3
		    DATA_SUBC, // 62 == -2
		    DATA_SUBC}; // 63 == -1

const cvec PILOT_VALUES = "1+1i 1-1i -1-1i 1+1i";
const ivec PILOT_LOCATIONS =  "7 21 43 57";

const cvec PREAMBLE_TONES=cvec("0.408248+0.588625i,"
			       "-0.537658-0.288675i,"
			       "0.632392+0.523747i,"
			       "0.000000+0.488725i,"
			       "0.278839+0.288675i,"
			       "-0.891211-0.601097i,"
			       "0.408248+0.377300i,"
			       "-1.543623+0.288675i,"
			       "-0.074715-0.264928i,"
			       "0.816497-1.454651i,"
			       "-0.428268-0.288675i,"
			       "0.931251+0.342278i,"
			       "0.612372+0.496874i,"
			       "-0.537658+2.055748i,"
			       "0.632392+0.431995i,"
			       "0.204124-0.377300i,"
			       "-0.836516+0.084551i,"
			       "-0.891211+0.264928i,"
			       "-0.204124+1.285549i,"
			       "-1.244765-0.831003i,"
			       "0.224144+0.643320i,"
			       "0.204124-0.588625i,"
			       "-0.428268-0.492799i,"
			       "-0.184104-0.523747i,"
			       "0.408248+0.588625i,"
			       "-0.537658-0.288675i,"
			       "0.632392+0.523747i,"
			       "0.000000+0.488725i,"
			       "0.278839+0.288675i,"
			       "-0.891211-0.601097i,"
			       "0.408248+0.377300i,"
			       "-1.543623+0.288675i,"
			       "-0.074715-0.264928i,"
			       "0.816497-1.454651i,"
			       "-0.428268-0.288675i,"
			       "0.931251+0.342278i,"
			       "0.612372+0.496874i,"
			       "-0.537658+2.055748i,"
			       "0.632392+0.431995i,"
			       "0.204124-0.377300i,"
			       "-0.836516+0.084551i,"
			       "-0.891211+0.264928i,"
			       "-0.204124+1.285549i,"
			       "-1.244765-0.831003i,"
			       "0.224144+0.643320i,"
			       "0.204124-0.588625i,"
			       "-0.428268-0.492799i,"
			       "-0.184104-0.523747i");

const bvec special_estimation_seq = "0 1 0 0 1 0 1 0 1 0 0 0 0 0 1 0 0 1 1 0 1 1 0 0 1 0 1 0 1 1 0 1 0 0 0 1 0 0 0 1 0 1 0 1 1 1 0 1 0 0 1 1 0 0 0 0 1 0 0 1 1 1 0 0";
const bvec short_ofdm_symbol_data = "1 0 1 0 0 0 0 0 1 0 0 1 1 0 1 1 0 0 1 0 1 0 1 0";
const int short_mask[] = {ZERO_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC, ZERO_SUBC, ZERO_SUBC, ZERO_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC, DATA_SUBC};

#define FOURTH_POWER_WINDOW 48

#define NBITS 1024

#endif
