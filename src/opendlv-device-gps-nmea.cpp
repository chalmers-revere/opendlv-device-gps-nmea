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
#include "opendlv-standard-message-set.hpp"

#include "nmea-decoder.hpp"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{0};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( (0 == commandlineArguments.count("nmea_ip")) || (0 == commandlineArguments.count("nmea_port")) || (0 == commandlineArguments.count("cid")) ) {
        std::cerr << argv[0] << " decodes latitude/longitude/heading from a Trimble GPS/INSS unit in NMEA format and publishes it to a running OpenDaVINCI session using the OpenDLV Standard Message Set." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --nmea_ip=<IPv4-address> --nmea_port=<port> --cid=<OpenDaVINCI session> [--id=<Identifier in case of multiple OxTS units>] [--verbose]" << std::endl;
        std::cerr << "Example: " << argv[0] << " --nmea_ip=10.42.42.112 --nmea_port=9999 --cid=111" << std::endl;
        retCode = 1;
    } else {
        const uint32_t ID{(commandlineArguments["id"].size() != 0) ? static_cast<uint32_t>(std::stoi(commandlineArguments["id"])) : 0};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        // Interface to a running OpenDaVINCI session (ignoring any incoming Envelopes).
        cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"])),
            [](auto){}
        };

        NMEADecoder nmeaDecoder{
            [&od4Session = od4, senderStamp = ID, VERBOSE](const double &latitude, const double &longitude) {
                opendlv::proxy::GeodeticWgs84Reading m;
                m.latitude(latitude).longitude(longitude);
                od4Session.send(m, cluon::data::TimeStamp(), senderStamp);

                // Print values on console.
                if (VERBOSE) {
                    std::stringstream buffer;
                    m.accept([](uint32_t, const std::string &, const std::string &) {},
                             [&buffer](uint32_t, std::string &&, std::string &&n, auto v) { buffer << n << " = " << v << '\n'; },
                             []() {});
                    std::cout << buffer.str() << std::endl;
                }
            },
            [&od4Session = od4, senderStamp = ID, VERBOSE](const float &heading) {
                opendlv::proxy::GeodeticHeadingReading m;
                m.northHeading(heading);
                od4Session.send(m, cluon::data::TimeStamp(), senderStamp);

                // Print values on console.
                if (VERBOSE) {
                    std::stringstream buffer;
                    m.accept([](uint32_t, const std::string &, const std::string &) {},
                             [&buffer](uint32_t, std::string &&, std::string &&n, auto v) { buffer << n << " = " << v << '\n'; },
                             []() {});
                    std::cout << buffer.str() << std::endl;
                }
            }
        };

        // Interface to a Trimble unit providing data in NMEA format.
        const std::string NMEA_ADDRESS(commandlineArguments["nmea_ip"]);
        const uint16_t NMEA_PORT(std::stoi(commandlineArguments["nmea_port"]));

        cluon::TCPConnection fromDevice{NMEA_ADDRESS, NMEA_PORT,
            [&decoder = nmeaDecoder](std::string &&d, std::chrono::system_clock::time_point &&tp) noexcept {
                cluon::data::TimeStamp sampleTime = cluon::time::convert(tp);
                (void)sampleTime;
                decoder.decode(d);
            },
            [&argv](){ std::cerr << "[" << argv[0] << "] Connection lost." << std::endl; exit(1); }
        };

        // Just sleep as this microservice is data driven.
        using namespace std::literals::chrono_literals;
        while (od4.isRunning()) {
            std::this_thread::sleep_for(1s);
        }
    }
    return retCode;
}
