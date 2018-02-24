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

#include "nmea-decoder.hpp"

#include <string>
#include <vector>

TEST_CASE("Test NMEADecoder with empty payload.") {
    bool latLonCalled{false};
    bool headingCalled{false};

    const std::string DATA;

    NMEADecoder d{
        [&latLonCalled](const double&, const double&){ latLonCalled = true; },
        [&headingCalled](const float&){ headingCalled = true; }
    };
    d.decode(DATA);

    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);
}

TEST_CASE("Test NMEADecoder with faulty payload.") {
    bool latLonCalled{false};
    bool headingCalled{false};

    const std::string DATA{"Hello World"};

    NMEADecoder d{
        [&latLonCalled](const double&, const double&){ latLonCalled = true; },
        [&headingCalled](const float&){ headingCalled = true; }
    };
    d.decode(DATA);

    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);
}

TEST_CASE("Test NMEADecoder with single sample payload.") {
    const std::string GGA{"$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    bool latLonCalled{false};
    bool headingCalled{false};
    double latitude{0};
    double longitude{0};

    NMEADecoder d{
        [&latLonCalled, &latitude, &longitude](const double &lat, const double &lon){ latLonCalled = true; latitude = lat; longitude = lon; },
        [&headingCalled](const float&){ headingCalled = true; }
    };
    d.decode(GGA);

    REQUIRE(latLonCalled);
    REQUIRE(!headingCalled);

    REQUIRE(37.391098 == Approx(latitude));
    REQUIRE(-122.037826 == Approx(longitude));
}

TEST_CASE("Test NMEADecoder with two consecutive sample payloads.") {
    const std::string GGA1{"$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};
    const std::string GGA2{"$GPGGA,172814.0,3723.46587704,S,12202.26957864,E,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    bool latLonCalled1{false};
    bool latLonCalled2{false};
    bool headingCalled{false};
    double latitude1{0};
    double longitude1{0};
    double latitude2{0};
    double longitude2{0};

    NMEADecoder d{
        [&latLonCalled1, &latitude1, &longitude1, &latLonCalled2, &latitude2, &longitude2](const double &lat, const double &lon){ 
            if (latLonCalled1 && !latLonCalled2) {
                latLonCalled2 = true; latitude2 = lat; longitude2 = lon;
            }
            if (!latLonCalled1 && !latLonCalled2) {
                latLonCalled1 = true; latitude1 = lat; longitude1 = lon;
            }
         },
        [&headingCalled](const float&){ headingCalled = true; }
    };
    {
        d.decode(GGA1);

        REQUIRE(latLonCalled1);
        REQUIRE(!latLonCalled2);
        REQUIRE(!headingCalled);

        REQUIRE(37.391098 == Approx(latitude1));
        REQUIRE(-122.037826 == Approx(longitude1));
    }

    {
        d.decode(GGA2);

        REQUIRE(latLonCalled1);
        REQUIRE(latLonCalled2);
        REQUIRE(!headingCalled);

        REQUIRE(-37.391098 == Approx(latitude2));
        REQUIRE(122.037826 == Approx(longitude2));
    }
}

TEST_CASE("Test NMEADecoder with sample payload with leading and trailing junk.") {
    const std::string GGA{"*4F\r\n$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n$GPGGA,172814.0"};

    bool latLonCalled{false};
    bool headingCalled{false};
    double latitude{0};
    double longitude{0};

    NMEADecoder d{
        [&latLonCalled, &latitude, &longitude](const double &lat, const double &lon){ latLonCalled = true; latitude = lat; longitude = lon; },
        [&headingCalled](const float&){ headingCalled = true; }
    };
    d.decode(GGA);

    REQUIRE(latLonCalled);
    REQUIRE(!headingCalled);

    REQUIRE(37.391098 == Approx(latitude));
    REQUIRE(-122.037826 == Approx(longitude));
}

TEST_CASE("Test NMEADecoder with fragmented sample payload with leading and trailing junk.") {
    const std::string GGA1{"*4F\r\n$GPGGA,172814.0,3723."};
    const std::string GGA2{"46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25"};
    const std::string GGA3{".669,M,2.0,0031*4F\r\n$GPGGA,172814.0"};

    bool latLonCalled{false};
    bool headingCalled{false};
    double latitude{0};
    double longitude{0};

    NMEADecoder d{
        [&latLonCalled, &latitude, &longitude](const double &lat, const double &lon){ latLonCalled = true; latitude = lat; longitude = lon; },
        [&headingCalled](const float&){ headingCalled = true; }
    };

    d.decode(GGA1);
    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);

    d.decode(GGA2);
    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);

    d.decode(GGA3);
    REQUIRE(latLonCalled);
    REQUIRE(!headingCalled);

    REQUIRE(37.391098 == Approx(latitude));
    REQUIRE(-122.037826 == Approx(longitude));
}

TEST_CASE("Test NMEADecoder with two fragmented sample payloads with leading and trailing junk.") {
    const std::string GGA1{"*4F\r\n$GPGGA,172814.0,3723."};
    const std::string GGA2{"46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25"};
    const std::string GGA3{".669,M,2.0,0031*4F\r\n$GPGGA,172814.0,3823.46587704,S,12302.26957864,E,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    bool latLonCalled1{false};
    bool latLonCalled2{false};
    bool headingCalled{false};
    double latitude1{0};
    double longitude1{0};
    double latitude2{0};
    double longitude2{0};

    NMEADecoder d{
        [&latLonCalled1, &latitude1, &longitude1, &latLonCalled2, &latitude2, &longitude2](const double &lat, const double &lon){ 
            if (latLonCalled1 && !latLonCalled2) {
                latLonCalled2 = true; latitude2 = lat; longitude2 = lon;
            }
            if (!latLonCalled1 && !latLonCalled2) {
                latLonCalled1 = true; latitude1 = lat; longitude1 = lon;
            }
         },
        [&headingCalled](const float&){ headingCalled = true; }
    };

    d.decode(GGA1);
    REQUIRE(!latLonCalled1);
    REQUIRE(!latLonCalled2);
    REQUIRE(!headingCalled);

    d.decode(GGA2);
    REQUIRE(!latLonCalled1);
    REQUIRE(!latLonCalled2);
    REQUIRE(!headingCalled);

    d.decode(GGA3);
    REQUIRE(latLonCalled1);
    REQUIRE(latLonCalled2);
    REQUIRE(!headingCalled);

    REQUIRE(37.391098 == Approx(latitude1));
    REQUIRE(-122.037826 == Approx(longitude1));
    REQUIRE(-38.391098 == Approx(latitude2));
    REQUIRE(123.037826 == Approx(longitude2));
}

