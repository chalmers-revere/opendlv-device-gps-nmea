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

#include "cluon-complete.hpp"
#include "nmea-decoder.hpp"
#include "nmea-decoder-constants.hpp"

#include <cmath>
#include <cstring>
#include <string>

NMEADecoder::NMEADecoder(
    std::function<void(const double &latitude, const double &longitude, const std::chrono::system_clock::time_point &tp)> delegateLatitudeLongitude,
    std::function<void(const float &heading, const std::chrono::system_clock::time_point &tp)> delegateHeading
    ) noexcept
    : m_delegateLatitudeLongitude(std::move(delegateLatitudeLongitude))
    , m_delegateHeading(std::move(delegateHeading)) {
    m_buffer = new uint8_t[NMEADecoderConstants::BUFFER_SIZE];
}

NMEADecoder::~NMEADecoder() {
    delete [] m_buffer;
    m_buffer = nullptr;
}

void NMEADecoder::decode(const std::string &data, std::chrono::system_clock::time_point &&tp) noexcept {
    const size_t bytesAvailable{data.size()};
    size_t bytesCopied{0};

    while (bytesCopied != bytesAvailable) {
      // How many bytes can we store in our buffer?
      size_t bytesToCopy{((NMEADecoderConstants::BUFFER_SIZE - m_size) < bytesAvailable) ? (NMEADecoderConstants::BUFFER_SIZE - m_size) : bytesAvailable};
      std::memcpy(m_buffer+m_size, data.data(), bytesToCopy);
      // Store how much we copied.
      bytesCopied = bytesToCopy;
      // Booking for the m_buffer fill level.
      m_size += bytesCopied;
      // Consume data from m_buffer.
      size_t consumed = parseBuffer(m_buffer, m_size, std::move(tp));
      // Discard processed entries.
      for (size_t i{0}; i < (m_size - consumed); i++) {
          m_buffer[i] = m_buffer[i + consumed];
      }
      m_size -= consumed;
      // If the parser does not work at all, cancel it.
      if (m_size >= NMEADecoderConstants::BUFFER_SIZE) {
          break;
      }
    }
}

size_t NMEADecoder::parseBuffer(const uint8_t *buffer, const size_t size, std::chrono::system_clock::time_point &&tp) {
    auto findCRLF = [](const uint8_t *_buffer, const size_t _offset, const size_t _size){
        size_t length{0};
        for(size_t i{0}; (_offset + NMEADecoderConstants::HEADER_SIZE + i + 1) < _size; i++) {
            if ( ('\r' == _buffer[_offset + NMEADecoderConstants::HEADER_SIZE + i]) &&
                ('\n' == _buffer[_offset + NMEADecoderConstants::HEADER_SIZE + i + 1]) ) {
                length = NMEADecoderConstants::HEADER_SIZE + i + 1;
                break;
            }
        }
        return length;
    };

    const std::chrono::system_clock::time_point timestamp{std::move(tp)};
    size_t offset{0};
    while (true) {
        // Sanity check whether we consumed all data.
        if ((offset + NMEADecoderConstants::HEADER_SIZE) > size) {
            return size;
        }

        if ( ('G' == buffer[offset + 3]) &&
             ('G' == buffer[offset + 4]) &&
             ('A' == buffer[offset + 5]) ) {
            // Search for CRLF to determine payload length.
            size_t length{findCRLF(buffer, offset, size)};
            if (0 == length) {
                // CRLF not found; need more data.
                return offset;
            }
            // Found CRLF; decode message.
            {
                std::string nmeaMessage(reinterpret_cast<char*>(m_buffer+offset), length);
                auto fields = stringtoolbox::split(nmeaMessage, ',');
                if (5 < fields.size()) {
                    double latitude = std::stod(fields[2]) / 100.0;
                    double longitude = std::stod(fields[4]) / 100.0;

                    latitude = static_cast<int32_t>(latitude) + (latitude - static_cast<int32_t>(latitude)) * 100.0 / 60.0;
                    longitude = static_cast<int32_t>(longitude) + (longitude - static_cast<int32_t>(longitude)) * 100.0 / 60.0;

                    latitude *= ("S" == fields[3] ? -1.0 : 1.0);
                    longitude *= ("W" == fields[5] ? -1.0 : 1.0);

                    if (nullptr != m_delegateLatitudeLongitude) {
                        m_delegateLatitudeLongitude(latitude, longitude, timestamp);
                    }
                }
            }
            offset += length;
        }
        else if ( ('R' == buffer[offset + 3]) &&
                  ('M' == buffer[offset + 4]) &&
                  ('C' == buffer[offset + 5]) ) {
            // Search for CRLF to determine payload length.
            size_t length{findCRLF(buffer, offset, size)};
            if (0 == length) {
                // CRLF not found; need more data.
                return offset;
            }
            {
                std::string nmeaMessage(reinterpret_cast<char*>(m_buffer+offset), length);
                auto fields = stringtoolbox::split(nmeaMessage, ',');
                if (8 < fields.size()) {
                    double latitude = std::stod(fields[3]) / 100.0;
                    double longitude = std::stod(fields[5]) / 100.0;

                    latitude = static_cast<int32_t>(latitude) + (latitude - static_cast<int32_t>(latitude)) * 100.0 / 60.0;
                    longitude = static_cast<int32_t>(longitude) + (longitude - static_cast<int32_t>(longitude)) * 100.0 / 60.0;

                    latitude *= ("S" == fields[4] ? -1.0 : 1.0);
                    longitude *= ("W" == fields[6] ? -1.0 : 1.0);
                    if (nullptr != m_delegateLatitudeLongitude) {
                        m_delegateLatitudeLongitude(latitude, longitude, timestamp);
                    }

                    const float heading = static_cast<float>(std::stod(fields[8]) / 180.0 * M_PI);
                    if (nullptr != m_delegateHeading) {
                        m_delegateHeading(heading, timestamp);
                    }
                }
            }
            // Found CRLF; decode message.
            offset += length;
        }
        else {
            // No HEADER bytes found yet; consume one byte.
            offset++;
        }
    }
    // We should not get here as we will leave the state machine inside
    // the while loop at certain point. If we are still getting here,
    // we simply discard everything and start over.
    return size;
}

