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
#include "nmea-decoder-constants.hpp"

#include <sstream>
#include <string>
#include <vector>
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
    std::pair<bool, std::vector<std::pair<opendlv::proxy::GeodeticWgs84Reading, opendlv::proxy::GeodeticHeadingReading> > > decode(const std::string &data) noexcept;

   private:
    bool m_foundHeader{false};
    bool m_foundCRLF{false};
    bool m_buffering{false};
    uint32_t m_toRemove{0};
    std::stringstream m_buffer{};
    uint32_t m_arrayWriteIndex{0};
    std::array<char, static_cast<uint32_t>(NMEADecoderConstants::NMEA_BUFFER)> m_bufferForNextNMEAMessage{};
    NMEADecoderConstants m_nextNMEAMessage{NMEADecoderConstants::UNKNOWN};
};

#endif

