\
#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "geo.hpp"
#include "graph.hpp"
#include "routing.hpp"

struct Place {
    int id{};
    std::string name;
    std::string abbreviation;
    Coordinate coordinate;
};

class CampusMap {
public:
    //Load map vertices and geographic coordinates from CSV
    void loadPlaces(const std::string& filename) {
        std::ifstream input(filename);
        if (!input) {
            throw std::runtime_error("Unable to open places file: " + filename);
        }

        std::string line;
        std::getline(input, line);

        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }

            std::stringstream stream(line);
            std::string idText;
            std::string name;
            std::string abbreviation;
            std::string latText;
            std::string lonText;

            std::getline(stream, idText, ',');
            std::getline(stream, name, ',');
            std::getline(stream, abbreviation, ',');
            std::getline(stream, latText, ',');
            std::getline(stream, lonText, ',');

            Place place{
                std::stoi(idText),
                name,
                abbreviation,
                {std::stod(latText), std::stod(lonText)}
            };

            places_[place.id] = place;
            graph_.addVertex(place.id);
        }
    }

    //Build weighted walkable connections from CSV edge data
    void loadEdges(const std::string& filename) {
        std::ifstream input(filename);
        if (!input) {
            throw std::runtime_error("Unable to open edges file: " + filename);
        }

        std::string line;
        std::getline(input, line);

        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }

            std::stringstream stream(line);
            std::string fromText;
            std::string toText;
            std::string multiplierText;

            std::getline(stream, fromText, ',');
            std::getline(stream, toText, ',');
            std::getline(stream, multiplierText, ',');

            const int from = std::stoi(fromText);
            const int to = std::stoi(toText);
            const double multiplier = multiplierText.empty() ? 1.0 : std::stod(multiplierText);

            if (!places_.count(from) || !places_.count(to)) {
                throw std::runtime_error("Edge references an unknown place id.");
            }

            //Use Haversine distance as the base edge cost before the path multiplier
            const double directDistance =
                haversineMiles(places_.at(from).coordinate, places_.at(to).coordinate);

            graph_.addUndirectedEdge(from, to, directDistance * multiplier);
        }
    }

    const WeightedGraph<int, double>& graph() const {
        return graph_;
    }

    const Place& place(int id) const {
        return places_.at(id);
    }

    const std::unordered_map<int, Place>& places() const {
        return places_;
    }

    std::optional<int> findPlace(const std::string& query) const {
        const std::string normalizedQuery = normalize(query);

        for (const auto& [id, place] : places_) {
            if (normalize(place.abbreviation) == normalizedQuery) {
                return id;
            }
        }

        for (const auto& [id, place] : places_) {
            if (normalize(place.name).find(normalizedQuery) != std::string::npos) {
                return id;
            }
        }

        return std::nullopt;
    }

    double heuristicMiles(int from, int to) const {
        return haversineMiles(
            places_.at(from).coordinate,
            places_.at(to).coordinate
        );
    }

private:
    static std::string normalize(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text;
    }

    std::unordered_map<int, Place> places_;
    WeightedGraph<int, double> graph_;
};

struct MeetingPointResult {
    int placeId{-1};
    RouteResult<int> routeA;
    RouteResult<int> routeB;
    double totalDistance{std::numeric_limits<double>::infinity()};
    double worstDistance{std::numeric_limits<double>::infinity()};
};

//Choose a meeting point by minimizing the longer route, then total distance
inline MeetingPointResult findFairMeetingPoint(
    const CampusMap& campus,
    int startA,
    int startB) {

    MeetingPointResult best;

    for (const auto& [candidateId, _] : campus.places()) {
        const auto routeA = aStar(
            campus.graph(),
            startA,
            candidateId,
            [&campus](int from, int to) {
                return campus.heuristicMiles(from, to);
            }
        );

        const auto routeB = aStar(
            campus.graph(),
            startB,
            candidateId,
            [&campus](int from, int to) {
                return campus.heuristicMiles(from, to);
            }
        );

        if (!routeA.found() || !routeB.found()) {
            continue;
        }

        const double worst = std::max(routeA.distance, routeB.distance);
        const double total = routeA.distance + routeB.distance;

        if (worst < best.worstDistance ||
            (std::abs(worst - best.worstDistance) < 1e-12 &&
             total < best.totalDistance)) {
            best.placeId = candidateId;
            best.routeA = routeA;
            best.routeB = routeB;
            best.totalDistance = total;
            best.worstDistance = worst;
        }
    }

    return best;
}
