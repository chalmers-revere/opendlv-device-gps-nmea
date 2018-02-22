/*
 * Copyright (C) 2018  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NMEA_DECODER
#define NMEA_DECODER

#include "opendlv-standard-message-set.hpp"

#include <string>
#include <utility>

class NMEADecoder {
   private:
    NMEADecoder(const NMEADecoder &) = delete;
    NMEADecoder(NMEADecoder &&)      = delete;
    NMEADecoder &operator=(const NMEADecoder &) = delete;
    NMEADecoder &operator=(NMEADecoder &&) = delete;

   public:
    NMEADecoder() = default;
    ~NMEADecoder() = default;

   public:
    std::pair<bool, std::pair<opendlv::proxy::GeodeticWgs84Reading, opendlv::proxy::GeodeticHeadingReading> > decode(const std::string &data) noexcept;
};

#endif

