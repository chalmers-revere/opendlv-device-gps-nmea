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

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#include "nmea-decoder.hpp"

#include <string>
#include <vector>

TEST_CASE("Test NMEADecoder with empty payload.") {
    const std::string DATA;

    NMEADecoder d;
    auto retVal = d.decode(DATA);

    REQUIRE(!retVal.first);
}

TEST_CASE("Test NMEADecoder with faulty payload.") {
    const std::string DATA{"Hello World"};

    NMEADecoder d;
    auto retVal = d.decode(DATA);

    REQUIRE(!retVal.first);
}

TEST_CASE("Test NMEADecoder with single sample payload.") {
    const std::string GGA{"$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    NMEADecoder d;
    auto retVal = d.decode(GGA);

    REQUIRE(retVal.first);

    auto listOfGeodeticTupels = retVal.second;

    REQUIRE(!listOfGeodeticTupels.empty());
    REQUIRE(1 == listOfGeodeticTupels.size());

    auto msgs = listOfGeodeticTupels.front();
    opendlv::proxy::GeodeticWgs84Reading msg1 = msgs.first;
    opendlv::proxy::GeodeticHeadingReading msg2 = msgs.second;

    REQUIRE(37.391098 == Approx(msg1.latitude()));
    REQUIRE(-122.037826 == Approx(msg1.longitude()));

    REQUIRE(0 == Approx(msg2.northHeading()));
}

TEST_CASE("Test NMEADecoder with two consecutive sample payloads.") {
    const std::string GGA1{"$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};
    const std::string GGA2{"$GPGGA,172814.0,3723.46587704,S,12202.26957864,E,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    NMEADecoder d;
    {
        auto retVal = d.decode(GGA1);

        REQUIRE(retVal.first);

        auto listOfGeodeticTupels = retVal.second;

        REQUIRE(!listOfGeodeticTupels.empty());
        REQUIRE(1 == listOfGeodeticTupels.size());

        auto msgs = listOfGeodeticTupels.front();
        opendlv::proxy::GeodeticWgs84Reading msg1 = msgs.first;
        opendlv::proxy::GeodeticHeadingReading msg2 = msgs.second;

        REQUIRE(37.391098 == Approx(msg1.latitude()));
        REQUIRE(-122.037826 == Approx(msg1.longitude()));

        REQUIRE(0 == Approx(msg2.northHeading()));
    }

    {
        auto retVal = d.decode(GGA2);

        REQUIRE(retVal.first);

        auto listOfGeodeticTupels = retVal.second;

        REQUIRE(!listOfGeodeticTupels.empty());
        REQUIRE(1 == listOfGeodeticTupels.size());

        auto msgs = listOfGeodeticTupels.front();
        opendlv::proxy::GeodeticWgs84Reading msg1 = msgs.first;
        opendlv::proxy::GeodeticHeadingReading msg2 = msgs.second;

        REQUIRE(-37.391098 == Approx(msg1.latitude()));
        REQUIRE(122.037826 == Approx(msg1.longitude()));

        REQUIRE(0 == Approx(msg2.northHeading()));
    }
}

TEST_CASE("Test NMEADecoder with sample payload with leading and trailing junk.") {
    const std::string GGA{"*4F\r\n$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n$GPGGA,172814.0"};

    NMEADecoder d;
    auto retVal = d.decode(GGA);

    REQUIRE(retVal.first);

    auto listOfGeodeticTupels = retVal.second;

    REQUIRE(!listOfGeodeticTupels.empty());
    REQUIRE(1 == listOfGeodeticTupels.size());

    auto msgs = listOfGeodeticTupels.front();
    opendlv::proxy::GeodeticWgs84Reading msg1 = msgs.first;
    opendlv::proxy::GeodeticHeadingReading msg2 = msgs.second;

    REQUIRE(37.391098 == Approx(msg1.latitude()));
    REQUIRE(-122.037826 == Approx(msg1.longitude()));

    REQUIRE(0 == Approx(msg2.northHeading()));
}

TEST_CASE("Test NMEADecoder with fragmented sample payload with leading and trailing junk.") {
    const std::string GGA1{"*4F\r\n$GPGGA,172814.0,3723."};
    const std::string GGA2{"46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25"};
    const std::string GGA3{".669,M,2.0,0031*4F\r\n$GPGGA,172814.0"};

    NMEADecoder d;

    auto retVal = d.decode(GGA1);
    REQUIRE(!retVal.first);

    retVal = d.decode(GGA2);
    REQUIRE(!retVal.first);

    retVal = d.decode(GGA3);
    REQUIRE(retVal.first);

    auto listOfGeodeticTupels = retVal.second;

    REQUIRE(!listOfGeodeticTupels.empty());
    REQUIRE(1 == listOfGeodeticTupels.size());

    auto msgs = listOfGeodeticTupels.front();
    opendlv::proxy::GeodeticWgs84Reading msg1 = msgs.first;
    opendlv::proxy::GeodeticHeadingReading msg2 = msgs.second;

    REQUIRE(37.391098 == Approx(msg1.latitude()));
    REQUIRE(-122.037826 == Approx(msg1.longitude()));

    REQUIRE(0 == Approx(msg2.northHeading()));
}

TEST_CASE("Test NMEADecoder with two fragmented sample payloads with leading and trailing junk.") {
    const std::string GGA1{"*4F\r\n$GPGGA,172814.0,3723."};
    const std::string GGA2{"46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25"};
    const std::string GGA3{".669,M,2.0,0031*4F\r\n$GPGGA,172814.0,3823.46587704,S,12302.26957864,E,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    NMEADecoder d;

    auto retVal = d.decode(GGA1);
    REQUIRE(!retVal.first);

    retVal = d.decode(GGA2);
    REQUIRE(!retVal.first);

    retVal = d.decode(GGA3);
    REQUIRE(retVal.first);

    auto listOfGeodeticTupels = retVal.second;

    REQUIRE(!listOfGeodeticTupels.empty());
    REQUIRE(2 == listOfGeodeticTupels.size());

    auto msgs = listOfGeodeticTupels.front();
    opendlv::proxy::GeodeticWgs84Reading msg1 = msgs.first;
    opendlv::proxy::GeodeticHeadingReading msg2 = msgs.second;

    REQUIRE(37.391098 == Approx(msg1.latitude()));
    REQUIRE(-122.037826 == Approx(msg1.longitude()));

    REQUIRE(0 == Approx(msg2.northHeading()));

    msgs = listOfGeodeticTupels.back();
    msg1 = msgs.first;
    msg2 = msgs.second;

    REQUIRE(-38.391098 == Approx(msg1.latitude()));
    REQUIRE(123.037826 == Approx(msg1.longitude()));

    REQUIRE(0 == Approx(msg2.northHeading()));
}

