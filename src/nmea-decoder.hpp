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

#include <chrono>
#include <functional>
#include <string>

class NMEADecoder {
   private:
    NMEADecoder(const NMEADecoder &) = delete;
    NMEADecoder(NMEADecoder &&)      = delete;
    NMEADecoder &operator=(const NMEADecoder &) = delete;
    NMEADecoder &operator=(NMEADecoder &&) = delete;

   public:
    NMEADecoder(std::function<void(const double &latitude, const double &longitude, const std::chrono::system_clock::time_point &tp)> delegateLatitudeLongitude,
                std::function<void(const float &heading, const std::chrono::system_clock::time_point &tp)> delegateHeading,
                std::function<void(const float &speed, const std::chrono::system_clock::time_point &tp)> delegateSpeed) noexcept;
    ~NMEADecoder();

   public:
    void decode(const std::string &data, std::chrono::system_clock::time_point &&tp) noexcept;

   private:
    size_t parseBuffer(const uint8_t *buffer, const size_t size, std::chrono::system_clock::time_point &&tp);

   private:
    uint8_t *m_buffer{nullptr};
    size_t m_size{0};

   private:
    std::function<void(const double &latitude, const double &longitude, const std::chrono::system_clock::time_point &tp)> m_delegateLatitudeLongitude{};
    std::function<void(const float &heading, const std::chrono::system_clock::time_point &tp)> m_delegateHeading{};
    std::function<void(const float &speed, const std::chrono::system_clock::time_point &tp)> m_delegateSpeed{};
};

#endif

