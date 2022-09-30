//
// Created by Ruslan Jonusas on 18/09/2022.
//

#ifndef NETWORK_MONITOR_TRANSPORT_NETWORK_H
#define NETWORK_MONITOR_TRANSPORT_NETWORK_H

#include <string>
#include <unordered_map>
#include <vector>

namespace NetworkMonitor {

    template<class T>
    using UP = std::unique_ptr<T>;

    template<class T>
    using SP = std::shared_ptr<T>;

    template<class T>
    using Vec = std::vector<T>;

    /*! \brief A station, line, or route ID.
 */
    using Id = std::string;
    using LL_INT = long long int;

    /*! \brief Network station
 *
 *  A Station struct is well formed if:
 *  - `id` is unique across all stations in the network.
 */
    struct Station {
        Id id{};
        std::string name{};

        /*! \brief Station comparison
     *
     *  Two stations are "equal" if they have the same ID.
     */
        bool operator==(const Station &other) const;
    };

    /*! \brief Network route
 *
 *  Each underground line has one or more edges. A route represents a single
 *  possible journey across a set of stops in a specified direction.
 *
 *  There may or may not be a corresponding route in the opposite direction of
 *  travel.
 *
 *  A Route struct is well formed if:
 *  - `id` is unique across all lines and their edges in the network.
 *  - The `lineId` line exists and has this route among its edges.
 *  - `stops` has at least 2 stops.
 *  - `startStationId` is the first stop in `stops`.
 *  - `endStationId` is the last stop in `stops`.
 *  - Every `stationId` station in `stops` exists.
 *  - Every stop in `stops` appears only once.
 */
    struct Route {
        Id id{};
        std::string direction{};
        Id lineId{};
        Id startStationId{};
        Id endStationId{};
        std::vector<Id> stops{};

        /*! \brief Route comparison
     *
     *  Two edges are "equal" if they have the same ID.
     */
        bool operator==(const Route &other) const;
    };

    /*! \brief Network line
 *
 *  A line is a collection of edges serving multiple stations.
 *
 *  A Line struct is well formed if:
 *  - `id` is unique across all lines in the network.
 *  - `edges` has at least 1 route.
 *  - Every route in `edges` is well formed.
 *  - Every route in `edges` has a `lineId` that is equal to this line `id`.
 */
    struct Line {
        Id id{};
        std::string name{};
        std::vector<Route> routes{};

        /*! \brief Line comparison
     *
     *  Two lines are "equal" if they have the same ID.
     */
        bool operator==(const Line &other) const;
    };

    /*! \brief Passenger event
 */
    struct PassengerEvent {
        enum class Type {
            In,
            Out
        };

        Id stationId{};
        Type type{Type::In};
    };

    /*! \brief Underground network representation
 */
    class TransportNetwork {

    private:


        struct StationNode;
        struct RouteEdge;
        struct RouteInternal;
        struct LineInternal;

        using Edges = std::vector<SP<RouteEdge>>;


        struct RouteEdge {
            SP<RouteInternal> route{nullptr};
            SP<StationNode> nextStop{nullptr};
            unsigned int travelTime{0};
        };

        struct StationNode {
            Id stationId;
            std::string name;
            LL_INT passengerCount{0};
            Edges edges{};

            [[nodiscard]] std::vector<SP<RouteEdge>>::const_iterator FindEdgeForRoute(const SP<RouteInternal> &route) const;
        };

        // Internal route representation
        struct RouteInternal {
            Id id{};
            Vec<SP<LineInternal>> line{nullptr};
            Vec<SP<StationNode>> stops{};
        };

        // Internal line representation
        // We map line edges by their ID.
        struct LineInternal {
            Id id{};
            std::string name{};
            std::unordered_map<Id, SP<RouteInternal>> routes{};
        };


    public:
        /*! \brief Default constructor
     */

        using Routes = std::vector<RouteEdge>;
        using Lines = std::unordered_map<Id, SP<LineInternal>>;
        using Stations = std::unordered_map<Id, SP<StationNode>>;

        TransportNetwork();
//
//        template<class R, class L, class S>
//        TransportNetwork(R &&edges, L &&lines, S &&stations) : routes_{std::forward(edges)},
//                                                                stations_{std::forward(stations)} {};

        /*! \brief Destructor
     */
        ~TransportNetwork();

        /*! \brief Copy constructor
     */
        TransportNetwork(
                const TransportNetwork &copied);

        /*! \brief Move constructor
     */
        TransportNetwork(
                TransportNetwork &&moved) noexcept ;

        /*! \brief Copy assignment operator
     */
        TransportNetwork &operator=(
                const TransportNetwork &copied);

        /*! \brief Move assignment operator
     */
        TransportNetwork &operator=(
                TransportNetwork &&moved);

        /*! \brief Add a station to the network.
     *
     *  \returns false if there was an error while adding the station to the
     *           network.
     *
     *  This function assumes that the Station object is well-formed.
     *
     *  The station cannot already be in the network.
     */
        bool AddStation(
                const Station &station);

        /*! \brief Add a line to the network.
     *
     *  \returns false if there was an error while adding the station to the
     *           network.
     *
     *  This function assumes that the Line object is well-formed.
     *
     *  All stations served by this line must already be in the network. The
     *  line cannot already be in the network.
     */
        bool AddLine(
                const Line &line);


        SP<StationNode> getStation(const Id& stationId) const;

        /*! \brief Record a passenger event at a station.
     *
     *  \returns false if the station is not in the network or if the passenger
     *           event is not reconized.
     */
        bool RecordPassengerEvent(
                const PassengerEvent &event) const;

        /*! \brief Get the number of passengers currently recorded at a station.
     *
     *  The returned number can be negative: This happens if we start recording
     *  in the middle of the day and we record more exiting than entering
     *  passengers.
     *
     *  \throws std::runtime_error if the station is not in the network.
     */
        long long int GetPassengerCount(
                const Id &station) const;

        /*! \brief Get list of edges serving a given station.
     *
     *  \returns An empty vector if there was an error getting the list of
     *           edges serving the station, or if the station has legitimately
     *           no edges serving it.
     *
     *  The station must already be in the network.
     */
        std::vector<Id> GetRoutesServingStation(
                const Id &stationId) const;

        /*! \brief Set the travel time between 2 adjacent stations.
     *
     *  \returns false if there was an error while setting the travel time
     *           between the two stations.
     *
     *  The travel time is the same for all edges connecting the two stations
     *  directly.
     *
     *  The two stations must be adjacent in at least one line route. The two
     *  stations must already be in the network.
     */
        bool SetTravelTime(
                const Id &stationA,
                const Id &stationB,
                const unsigned int travelTime);

        /*! \brief Get the travel time between 2 adjacent stations.
     *
     *  \returns 0 if the function could not find the travel time between the
     *           two stations, or if station A and B are the same station.
     *
     *  The travel time is the same for all edges connecting the two stations
     *  directly.
     *
     *  The two stations must be adjacent in at least one line route. The two
     *  stations must already be in the network.
     */
        unsigned int GetTravelTime(
                const Id &stationA,
                const Id &stationB) const;

        /*! \brief Get the total travel time between any 2 stations, on a specific
     *         route.
     *
     *  The total travel time is the cumulative sum of the travel times between
     *  all stations between `stationA` and `stationB`.
     *
     *  \returns 0 if the function could not find the travel time between the
     *           two stations, or if station A and B are the same station.
     *
     *  The two stations must be both served by the `route`. The two stations
     *  must already be in the network.
     */
        unsigned int GetTravelTime(
                const Id &line,
                const Id &route,
                const Id &stationA,
                const Id &stationB) const;

        Lines lines_;
        Stations stations_;
    };

}// namespace NetworkMonitor

#endif//NETWORK_MONITOR_TRANSPORT_NETWORK_H
