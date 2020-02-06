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

#include <chrono>
#include <string>
#include <vector>

TEST_CASE("Test NMEADecoder with empty payload.") {
    bool latLonCalled{false};
    bool headingCalled{false};
    bool speedCalled{false};

    const std::string DATA;

    NMEADecoder d{
        [&latLonCalled](const double&, const double&, const std::chrono::system_clock::time_point &){ latLonCalled = true; },
        [&headingCalled](const float&, const std::chrono::system_clock::time_point &){ headingCalled = true; },
        [&speedCalled](const float&, const std::chrono::system_clock::time_point &){ speedCalled = true; }
    };
    d.decode(DATA, std::chrono::system_clock::time_point());

    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);
}

TEST_CASE("Test NMEADecoder with faulty payload.") {
    bool latLonCalled{false};
    bool headingCalled{false};
    bool speedCalled{false};

    const std::string DATA{"Hello World"};

    NMEADecoder d{
        [&latLonCalled](const double&, const double&, const std::chrono::system_clock::time_point &){ latLonCalled = true; },
        [&headingCalled](const float&, const std::chrono::system_clock::time_point &){ headingCalled = true; },
        [&speedCalled](const float&, const std::chrono::system_clock::time_point &){ speedCalled = true; }
    };
    d.decode(DATA, std::chrono::system_clock::time_point());

    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);
}

TEST_CASE("Test NMEADecoder with single sample GGA.") {
    const std::string GGA{"$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    bool latLonCalled{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude{0};
    double longitude{0};

    NMEADecoder d{
        [&latLonCalled, &latitude, &longitude](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ latLonCalled = true; latitude = lat; longitude = lon; },
        [&headingCalled](const float&, const std::chrono::system_clock::time_point &){ headingCalled = true; },
        [&speedCalled](const float&, const std::chrono::system_clock::time_point &){ speedCalled = true; }
    };
    d.decode(GGA, std::chrono::system_clock::time_point());

    REQUIRE(latLonCalled);
    REQUIRE(!headingCalled);

    REQUIRE(37.391098 == Approx(latitude));
    REQUIRE(-122.037826 == Approx(longitude));
    REQUIRE(!speedCalled);
}

TEST_CASE("Test NMEADecoder with single sample RMC.") {
    const std::string RMC{"$GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68\r\n"};

    bool latLonCalled{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude{0};
    double longitude{0};
    float heading{0.0f};
    float speed{0.0f};

    NMEADecoder d{
        [&latLonCalled, &latitude, &longitude](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ latLonCalled = true; latitude = lat; longitude = lon; },
        [&headingCalled, &heading](const float &h, const std::chrono::system_clock::time_point &){ headingCalled = true; heading = h;},
        [&speedCalled, &speed](const float &f, const std::chrono::system_clock::time_point &){ speedCalled = true; speed = f; }
    };
    d.decode(RMC, std::chrono::system_clock::time_point());

    REQUIRE(latLonCalled);
    REQUIRE(headingCalled);
    REQUIRE(speedCalled);

    REQUIRE(49.274167 == Approx(latitude));
    REQUIRE(-123.185333 == Approx(longitude));
    REQUIRE(0.95469f == Approx(heading));
    REQUIRE(0.25722f == Approx(speed));
}

TEST_CASE("Test NMEADecoder with two consecutive sample GGAs.") {
    const std::string GGA1{"$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};
    const std::string GGA2{"$GPGGA,172814.0,3723.46587704,S,12202.26957864,E,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    bool latLonCalled1{false};
    bool latLonCalled2{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude1{0};
    double longitude1{0};
    double latitude2{0};
    double longitude2{0};

    NMEADecoder d{
        [&latLonCalled1, &latitude1, &longitude1, &latLonCalled2, &latitude2, &longitude2](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ 
            if (latLonCalled1 && !latLonCalled2) {
                latLonCalled2 = true; latitude2 = lat; longitude2 = lon;
            }
            if (!latLonCalled1 && !latLonCalled2) {
                latLonCalled1 = true; latitude1 = lat; longitude1 = lon;
            }
         },
        [&headingCalled](const float&, const std::chrono::system_clock::time_point &){ headingCalled = true; },
        [&speedCalled](const float&, const std::chrono::system_clock::time_point &){ speedCalled = true; }
    };
    {
        d.decode(GGA1, std::chrono::system_clock::time_point());

        REQUIRE(latLonCalled1);
        REQUIRE(!latLonCalled2);
        REQUIRE(!headingCalled);
        REQUIRE(!speedCalled);

        REQUIRE(37.391098 == Approx(latitude1));
        REQUIRE(-122.037826 == Approx(longitude1));
    }

    {
        d.decode(GGA2, std::chrono::system_clock::time_point());

        REQUIRE(latLonCalled1);
        REQUIRE(latLonCalled2);
        REQUIRE(!headingCalled);
        REQUIRE(!speedCalled);

        REQUIRE(-37.391098 == Approx(latitude2));
        REQUIRE(122.037826 == Approx(longitude2));
    }
}

TEST_CASE("Test NMEADecoder with two consecutive sample GGA and RMC.") {
    const std::string GGA{"$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};
    const std::string RMC{"$GPRMC,225446,A,4916.45,N,12311.12,W,000.6,054.7,191194,020.3,E*68\r\n"};

    bool latLonCalled1{false};
    bool latLonCalled2{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude1{0};
    double longitude1{0};
    double latitude2{0};
    double longitude2{0};
    float heading{0.0f};
    float speed{0.0f};

    NMEADecoder d{
        [&latLonCalled1, &latitude1, &longitude1, &latLonCalled2, &latitude2, &longitude2](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ 
            if (latLonCalled1 && !latLonCalled2) {
                latLonCalled2 = true; latitude2 = lat; longitude2 = lon;
            }
            if (!latLonCalled1 && !latLonCalled2) {
                latLonCalled1 = true; latitude1 = lat; longitude1 = lon;
            }
         },
        [&headingCalled, &heading](const float &h, const std::chrono::system_clock::time_point &){ headingCalled = true; heading = h; },
        [&speedCalled, &speed](const float &s, const std::chrono::system_clock::time_point &){ speedCalled = true; speed = s;}
    };
    {
        d.decode(GGA, std::chrono::system_clock::time_point());

        REQUIRE(latLonCalled1);
        REQUIRE(!latLonCalled2);
        REQUIRE(!headingCalled);
        REQUIRE(!speedCalled);

        REQUIRE(37.391098 == Approx(latitude1));
        REQUIRE(-122.037826 == Approx(longitude1));
    }

    {
        d.decode(RMC, std::chrono::system_clock::time_point());

        REQUIRE(latLonCalled1);
        REQUIRE(latLonCalled2);
        REQUIRE(headingCalled);
        REQUIRE(speedCalled);

        REQUIRE(49.274167 == Approx(latitude2));
        REQUIRE(-123.185333 == Approx(longitude2));
        REQUIRE(0.95469f == Approx(heading));
        REQUIRE(0.3086664f == Approx(speed));
    }
}

TEST_CASE("Test NMEADecoder with sample GGA with leading and trailing junk.") {
    const std::string GGA{"*4F\r\n$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n$GPGGA,172814.0"};

    bool latLonCalled{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude{0};
    double longitude{0};

    NMEADecoder d{
        [&latLonCalled, &latitude, &longitude](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ latLonCalled = true; latitude = lat; longitude = lon; },
        [&headingCalled](const float&, const std::chrono::system_clock::time_point &){ headingCalled = true; },
        [&speedCalled](const float&, const std::chrono::system_clock::time_point &){ speedCalled = true; }
    };
    d.decode(GGA, std::chrono::system_clock::time_point());

    REQUIRE(latLonCalled);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    REQUIRE(37.391098 == Approx(latitude));
    REQUIRE(-122.037826 == Approx(longitude));
}

TEST_CASE("Test NMEADecoder with fragmented sample GGA with leading and trailing junk.") {
    const std::string GGA1{"*4F\r\n$GPGGA,172814.0,3723."};
    const std::string GGA2{"46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25"};
    const std::string GGA3{".669,M,2.0,0031*4F\r\n$GPGGA,172814.0"};

    bool latLonCalled{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude{0};
    double longitude{0};

    NMEADecoder d{
        [&latLonCalled, &latitude, &longitude](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ latLonCalled = true; latitude = lat; longitude = lon; },
        [&headingCalled](const float&, const std::chrono::system_clock::time_point &){ headingCalled = true; },
        [&speedCalled](const float&, const std::chrono::system_clock::time_point &){ speedCalled = true; }
    };

    d.decode(GGA1, std::chrono::system_clock::time_point());
    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    d.decode(GGA2, std::chrono::system_clock::time_point());
    REQUIRE(!latLonCalled);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    d.decode(GGA3, std::chrono::system_clock::time_point());
    REQUIRE(latLonCalled);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    REQUIRE(37.391098 == Approx(latitude));
    REQUIRE(-122.037826 == Approx(longitude));
}

TEST_CASE("Test NMEADecoder with two fragmented sample GGAs with leading junk.") {
    const std::string GGA1{"*4F\r\n$GPGGA,172814.0,3723."};
    const std::string GGA2{"46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25"};
    const std::string GGA3{".669,M,2.0,0031*4F\r\n$GPGGA,172814.0,3823.46587704,S,12302.26957864,E,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n"};

    bool latLonCalled1{false};
    bool latLonCalled2{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude1{0};
    double longitude1{0};
    double latitude2{0};
    double longitude2{0};

    NMEADecoder d{
        [&latLonCalled1, &latitude1, &longitude1, &latLonCalled2, &latitude2, &longitude2](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ 
            if (latLonCalled1 && !latLonCalled2) {
                latLonCalled2 = true; latitude2 = lat; longitude2 = lon;
            }
            if (!latLonCalled1 && !latLonCalled2) {
                latLonCalled1 = true; latitude1 = lat; longitude1 = lon;
            }
        },
        [&headingCalled](const float&, const std::chrono::system_clock::time_point &){ headingCalled = true; },
        [&speedCalled](const float&, const std::chrono::system_clock::time_point &){ speedCalled = true; }
    };

    d.decode(GGA1, std::chrono::system_clock::time_point());
    REQUIRE(!latLonCalled1);
    REQUIRE(!latLonCalled2);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    d.decode(GGA2, std::chrono::system_clock::time_point());
    REQUIRE(!latLonCalled1);
    REQUIRE(!latLonCalled2);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    d.decode(GGA3, std::chrono::system_clock::time_point());
    REQUIRE(latLonCalled1);
    REQUIRE(latLonCalled2);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    REQUIRE(37.391098 == Approx(latitude1));
    REQUIRE(-122.037826 == Approx(longitude1));
    REQUIRE(-38.391098 == Approx(latitude2));
    REQUIRE(123.037826 == Approx(longitude2));
}

TEST_CASE("Test NMEADecoder with two fragmented sample GGA and RMC with leading and trailing junk.") {
    const std::string DATA1{"*4F\r\n$GPGGA,172814.0,3723."};
    const std::string DATA2{"46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25"};
    const std::string DATA3{".669,M,2.0,0031*4F\r\n$GPRMC,225446,A,4916.45,N,12311.12,W,2,054.7"};
    const std::string DATA4{",191194,020.3,E*68\r\n$GPGGA,172814.0"};

    bool latLonCalled1{false};
    bool latLonCalled2{false};
    bool headingCalled{false};
    bool speedCalled{false};
    double latitude1{0};
    double longitude1{0};
    double latitude2{0};
    double longitude2{0};
    float heading{0.0f};
    float speed{0.0f};

    NMEADecoder d{
        [&latLonCalled1, &latitude1, &longitude1, &latLonCalled2, &latitude2, &longitude2](const double &lat, const double &lon, const std::chrono::system_clock::time_point &){ 
            if (latLonCalled1 && !latLonCalled2) {
                latLonCalled2 = true; latitude2 = lat; longitude2 = lon;
            }
            if (!latLonCalled1 && !latLonCalled2) {
                latLonCalled1 = true; latitude1 = lat; longitude1 = lon;
            }
        },
        [&headingCalled, &heading](const float &h, const std::chrono::system_clock::time_point &){ headingCalled = true; heading = h; },
        [&speedCalled, &speed](const float &s, const std::chrono::system_clock::time_point &){ speedCalled = true; speed = s;}
    };

    d.decode(DATA1, std::chrono::system_clock::time_point());
    REQUIRE(!latLonCalled1);
    REQUIRE(!latLonCalled2);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    d.decode(DATA2, std::chrono::system_clock::time_point());
    REQUIRE(!latLonCalled1);
    REQUIRE(!latLonCalled2);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    d.decode(DATA3, std::chrono::system_clock::time_point());
    REQUIRE(latLonCalled1);
    REQUIRE(!latLonCalled2);
    REQUIRE(!headingCalled);
    REQUIRE(!speedCalled);

    REQUIRE(37.391098 == Approx(latitude1));
    REQUIRE(-122.037826 == Approx(longitude1));

    d.decode(DATA4, std::chrono::system_clock::time_point());
    REQUIRE(latLonCalled1);
    REQUIRE(latLonCalled2);
    REQUIRE(headingCalled);
    REQUIRE(speedCalled);

    REQUIRE(49.274167 == Approx(latitude2));
    REQUIRE(-123.185333 == Approx(longitude2));
    REQUIRE(0.95469f == Approx(heading));
    REQUIRE(1.02888f == Approx(speed));
}

