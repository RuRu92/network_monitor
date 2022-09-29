//
// Created by Ruslan Jonusas on 18/09/2022.
//

#include "network-monitor/transport_network.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

using NetworkMonitor::Id;
using NetworkMonitor::Line;
using NetworkMonitor::PassengerEvent;
using NetworkMonitor::Route;
using NetworkMonitor::SP;
using NetworkMonitor::Station;
using NetworkMonitor::TransportNetwork;
using Stations = NetworkMonitor::TransportNetwork::Stations;
using Lines = NetworkMonitor::TransportNetwork::Lines;


bool NetworkMonitor::Station::operator==(const Station &other) const {
    return this->id == other.id;
}
bool NetworkMonitor::Route::operator==(const Route &other) const {
    return this->id == other.id;
}
bool NetworkMonitor::Line::operator==(const Line &other) const {
    return this->id == other.id;
}

// Transport Network

NetworkMonitor::TransportNetwork::TransportNetwork() = default;

NetworkMonitor::TransportNetwork::~TransportNetwork() = default;


bool NetworkMonitor::TransportNetwork::AddStation(const Station &station) {
    if (stations_[station.id] != nullptr) {
        return false;
    };



    return false;
};

bool NetworkMonitor::TransportNetwork::AddLine(const Line &line) {

    if (lines_.find(line.id) != lines_.end()) {
        return false;
    }

    auto lineInternal{
            std::make_shared<LineInternal>(LineInternal{
                    .id{line.id},
                    .name{line.name},
            })};


    std::unordered_map<Id, SP<RouteInternal>> routes{};
    std::vector<SP<StationNode>> stops{};
    std::for_each(line.routes.begin(), line.routes.end(), [&routes, &stops, &lineInternal, this](const Route &route) {
        stops.reserve(route.stops.size());
        for (const auto &stopId: route.stops) {
            const auto station{getStation(stopId)};
            if (station == nullptr) {
                return false;
            }
            std::cout << "Here in loop" << std::endl;
            stops.push_back(station);
        }

        auto rt{std::make_shared<RouteInternal>(RouteInternal{
                .id{route.id},
                .line{lineInternal},
                .stops{stops}})};

        for (size_t idx{0}; idx < rt->stops.size() - 1; ++idx) {
            auto currentStop = getStation(rt->stops[idx]->stationId);
            auto nextStop = getStation(rt->stops[idx + 1]->stationId);// find final stop wihout index out of bound

            currentStop->edges.emplace_back(std::make_shared<RouteEdge>(RouteEdge{
                    .route{rt},
                    .nextStop{nextStop},
                    .travelTime = 0}));
        }

        std::cout << "Adding internals full edges" << std::endl;
        routes.emplace(route.id, std::move(rt));
        routes = {};
        return true;
    });

    lineInternal->routes = routes;
    lines_.emplace(lineInternal->id, std::move(lineInternal));
    return true;
}

bool NetworkMonitor::TransportNetwork::RecordPassengerEvent(const PassengerEvent &event) const {
    auto station = getStation(event.stationId);
    if (station == nullptr) {
        return false;
    }

    station->passengerCount += 1;

    return true;
}

long long int NetworkMonitor::TransportNetwork::GetPassengerCount(const Id &stationId) const {
    auto stationIt{stations_.find(stationId)};

    if (stationIt == stations_.end()) {
        throw std::runtime_error("Could not find station in the network: " +
                                 stationId);
    };

    std::cout << "Iterator item: " << stationIt->first << std::endl;

    SP<StationNode> st = stationIt->second;

    return st->passengerCount;
}

std::vector<Id> NetworkMonitor::TransportNetwork::GetRoutesServingStation(const Id &stationId) const {
    auto station{getStation(stationId)};

    if (station == nullptr) {
        return {};
    }

    std::vector<Id> routes_ids{};

    std::for_each(station->edges.begin(), station->edges.end(), [&routes_ids](const SP<RouteEdge> &route) {
        routes_ids.emplace_back(route->route->id);
    });

    for (auto &[_, line]: lines_) {
        for (const auto &[_, route]: line->routes) {
            const auto &endStop{route->stops[route->stops.size() - 1]};
            if (endStop == station) {
                routes_ids.emplace_back(route->id);
            }
        }
    }

    return routes_ids;
}

bool NetworkMonitor::TransportNetwork::SetTravelTime(const Id &stationA, const Id &stationB, const unsigned int travelTime) {

    auto station1{getStation(stationA)};
    auto station2{getStation(stationA)};

    if (station1 == nullptr || station2 == nullptr) {
        return false;
    }

    bool foundAnyEdge{false};
    auto setTravelTimeFn = [&foundAnyEdge, &travelTime](auto from, auto to) {
        for (const auto &route: from->edges) {
            if (route->nextStop == to) {
                route->travelTime = travelTime;
                foundAnyEdge = true;
            }
        }
    };

    setTravelTimeFn(station1, station2);
    setTravelTimeFn(station2, station1);
    return foundAnyEdge;
};


SP<TransportNetwork::StationNode> NetworkMonitor::TransportNetwork::getStation(const Id &stationId) const {
    auto stationIt{stations_.find(stationId)};
    if (stationIt == stations_.end()) {
        return nullptr;
    }
    return stationIt->second;
}
TransportNetwork &NetworkMonitor::TransportNetwork::operator=(TransportNetwork &&moved) = default;
TransportNetwork &NetworkMonitor::TransportNetwork::operator=(const TransportNetwork &copied) = default;

TransportNetwork::TransportNetwork(const TransportNetwork &copied) = default;
NetworkMonitor::TransportNetwork::TransportNetwork(TransportNetwork &&moved) noexcept = default;

//
//TransportNetwork::StationNode(const Id &stationId,
//                                         const Edges &edges) : stationId_(stationId), edges(edges) {}

//
//std::vector<NetworkMonitor::SP<NetworkMonitor::TransportNetwork::RouteEdge>, std::allocator<NetworkMonitor::SP<NetworkMonitor::TransportNetwork::RouteEdge>>>::const_iterator NetworkMonitor::TransportNetwork::StationNode::FindEdgeForRoute(const NetworkMonitor::SP<NetworkMonitor::TransportNetwork::RouteInternal> &route) const {
//    return std::vector<NetworkMonitor::SP<NetworkMonitor::TransportNetwork::RouteEdge>, std::allocator<NetworkMonitor::SP<NetworkMonitor::TransportNetwork::RouteEdge>>>::const_iterator();
//}

std::vector<
        SP<TransportNetwork::RouteEdge>>::const_iterator
TransportNetwork::StationNode::FindEdgeForRoute(
        const std::shared_ptr<RouteInternal> &route) const {
    return std::find_if(
            edges.begin(),
            edges.end(),
            [&route](const auto &edge) {
                return edge->route == route;
            });
}
