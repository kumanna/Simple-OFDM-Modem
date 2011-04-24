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

#ifndef TRANSMIT_HPP
#define TRANSMIT_HPP

#include "parameters.hpp"

cvec
generate_special_estimation_sequence();

int
fill_bits_into_ofdm_symbols(const bvec &bits, QAM &qam, OFDM &ofdm, cvec &special_estimation_seq, int *packet_length, cvec &ofdm_syms);

#endif
