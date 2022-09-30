//
// Created by Ruslan Jonusas on 25/09/2022.
//

#include <network-monitor/transport_network.h>

#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>


using NetworkMonitor::Line;
using NetworkMonitor::PassengerEvent;
using NetworkMonitor::Route;
using NetworkMonitor::Station;
using EventType = NetworkMonitor::PassengerEvent::Type;
using NetworkMonitor::TransportNetwork;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(class_TransportNetwork);

BOOST_AUTO_TEST_SUITE(AddStation);

BOOST_AUTO_TEST_CASE(basic) {
    TransportNetwork nw{};
    bool ok{false};

    // Add a station.
    Station station{
            "station_000",
            "Station Name",
    };
    ok = nw.AddStation(station);

    BOOST_CHECK(nw.stations_.size() == 1);
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(duplicate_id) {
    TransportNetwork nw{};
    bool ok{false};

    // Can't add the same station twice.
    Station station{
            "station_000",
            "Station Name",
    };
    ok = nw.AddStation(station);
    BOOST_REQUIRE(ok);
    ok = nw.AddStation(station);
    BOOST_CHECK(!ok);
}

BOOST_AUTO_TEST_SUITE_END();// AddStation


BOOST_AUTO_TEST_SUITE(AddLine);

BOOST_AUTO_TEST_CASE(basic) {
    TransportNetwork nw{};
    bool ok{false};

    // Add a line with 1 route.
    // route0: 0 ---> 1

    // First, add the stations.
    Station station0{
            "station_000",
            "Station Name 0",
    };
    Station station1{
            "station_001",
            "Station Name 1",
    };
    ok = true;
    ok &= nw.AddStation(station0);
    ok &= nw.AddStation(station1);
    BOOST_REQUIRE(ok);

    // Then, add the line, with the two edges.
    Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station_000",
            "station_001",
            {"station_000", "station_001"},
    };
    Line line{
            "line_000",
            "Line Name",
            {route0},
    };
    ok = nw.AddLine(line);
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(shared_stations) {
    TransportNetwork nw{};
    bool ok{false};

    // Define a line with 2 edges going through some shared stations.
    // route0: 0 ---> 1 ---> 2
    // route1: 3 ---> 1 ---> 2
    Station station0{
            "station_000",
            "Station Name 0",
    };
    Station station1{
            "station_001",
            "Station Name 1",
    };
    Station station2{
            "station_002",
            "Station Name 2",
    };
    Station station3{
            "station_003",
            "Station Name 3",
    };
    Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station_000",
            "station_002",
            {"station_000", "station_001", "station_002"},
    };
    Route route1{
            "route_001",
            "inbound",
            "line_000",
            "station_003",
            "station_002",
            {"station_003", "station_001", "station_002"},
    };
    Line line{
            "line_000",
            "Line Name",
            {route0, route1},
    };
    ok = true;
    ok &= nw.AddStation(station0);
    ok &= nw.AddStation(station1);
    ok &= nw.AddStation(station2);
    ok &= nw.AddStation(station3);
    BOOST_REQUIRE(ok);
    ok = nw.AddLine(line);
    BOOST_CHECK(nw.lines_.size() == 1);
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(duplicate) {
    TransportNetwork nw{};
    bool ok{false};

    // Can't add the same line twice.
    Station station0{
            "station_000",
            "Station Name 0",
    };
    Station station1{
            "station_001",
            "Station Name 1",
    };
    Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station_000",
            "station_001",
            {"station_000", "station_001"},
    };
    Line line{
            "line_000",
            "Line Name",
            {route0},
    };
    ok = true;
    ok &= nw.AddStation(station0);
    ok &= nw.AddStation(station1);
    BOOST_REQUIRE(ok);
    ok = nw.AddLine(line);
    BOOST_REQUIRE(ok);
    ok = nw.AddLine(line);
    BOOST_CHECK(!ok);
}

BOOST_AUTO_TEST_SUITE_END();// AddLine

BOOST_AUTO_TEST_SUITE(PassengerEvents);

BOOST_AUTO_TEST_CASE(basic) {
    TransportNetwork nw{};
    bool ok{false};

    // Add a line with 1 route.
    // route0: 0 ---> 1 ---> 2
    Station station0{
            "station_000",
            "Station Name 0",
    };
    Station station1{
            "station_001",
            "Station Name 1",
    };
    Station station2{
            "station_002",
            "Station Name 2",
    };
    Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station_000",
            "station_002",
            {"station_000", "station_001", "station_002"},
    };
    Line line{
            "line_000",
            "Line Name",
            {route0},
    };
    ok = true;
    ok &= nw.AddStation(station0);
    ok &= nw.AddStation(station1);
    ok &= nw.AddStation(station2);
    BOOST_REQUIRE(ok);
    ok = nw.AddLine(line);
    BOOST_REQUIRE(ok);

    // Check that the network starts empty.
    BOOST_REQUIRE_EQUAL(nw.GetPassengerCount(station0.id), 0);
    BOOST_REQUIRE_EQUAL(nw.GetPassengerCount(station1.id), 0);
    BOOST_REQUIRE_EQUAL(nw.GetPassengerCount(station2.id), 0);
    try {
        auto count{nw.GetPassengerCount("station_42")};// Not in the network
        BOOST_REQUIRE(false);
    } catch (const std::runtime_error &e) {
        BOOST_REQUIRE(true);
    }

    // Record events and check the count.
    using EventType = PassengerEvent::Type;
    ok = nw.RecordPassengerEvent({station0.id, EventType::In});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station0.id), 1);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station1.id), 0);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station2.id), 0);
    ok = nw.RecordPassengerEvent({station0.id, EventType::In});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station0.id), 2);
    ok = nw.RecordPassengerEvent({station1.id, EventType::In});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station0.id), 2);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station1.id), 1);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station2.id), 0);
    ok = nw.RecordPassengerEvent({station0.id, EventType::Out});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station0.id), 1);
    ok = nw.RecordPassengerEvent({station2.id, EventType::Out});// Negative
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.GetPassengerCount(station2.id), -1);
}

BOOST_AUTO_TEST_SUITE_END();// GetPassengerCount

BOOST_AUTO_TEST_SUITE(GetRoutesServingStation);

BOOST_AUTO_TEST_CASE(basic)
{
    TransportNetwork nw {};
    bool ok {false};

    // Add a line with 1 route.
    // route0: 0 ---> 1 ---> 2
    // Plus a station served by no routes: 3.
    Station station0 {
            "station_000",
            "Station Name 0",
    };
    Station station1 {
            "station_001",
            "Station Name 1",
    };
    Station station2 {
            "station_002",
            "Station Name 2",
    };
    Station station3 {
            "station_003",
            "Station Name 3",
    };
    Route route0 {
            "route_000",
            "inbound",
            "line_000",
            "station_000",
            "station_002",
            {"station_000", "station_001", "station_002"},
    };
   Line line {
            "line_000",
            "Line Name",
            {route0},
    };
    ok = true;
    ok &= nw.AddStation(station0);
    ok &= nw.AddStation(station1);
    ok &= nw.AddStation(station2);
    ok &= nw.AddStation(station3);
    BOOST_REQUIRE(ok);
    ok = nw.AddLine(line);
    BOOST_REQUIRE(ok);

    // Check the routes served.
    std::vector<NetworkMonitor::Id> routes {};
    routes = nw.GetRoutesServingStation(station0.id);
    BOOST_REQUIRE_EQUAL(routes.size(), 1);
    BOOST_CHECK(routes[0] == route0.id);
    routes = nw.GetRoutesServingStation(station1.id);
    BOOST_REQUIRE_EQUAL(routes.size(), 1);
    BOOST_CHECK(routes[0] == route0.id);
    routes = nw.GetRoutesServingStation(station2.id);
    BOOST_REQUIRE_EQUAL(routes.size(), 1);
    BOOST_CHECK(routes[0] == route0.id);
    routes = nw.GetRoutesServingStation(station3.id);
    BOOST_CHECK_EQUAL(routes.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END(); // GetRoutesServingStation

BOOST_AUTO_TEST_SUITE_END();// class_TransportNetwork

BOOST_AUTO_TEST_SUITE_END();// websocket_client