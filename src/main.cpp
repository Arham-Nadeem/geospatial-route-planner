\
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "campus.hpp"

namespace {

void printPath(const CampusMap& campus, const RouteResult<int>& route) {
    if (!route.found()) {
        std::cout << "No route found.\n";
        return;
    }

    for (std::size_t i = 0; i < route.path.size(); ++i) {
        std::cout << campus.place(route.path[i]).name;
        if (i + 1 != route.path.size()) {
            std::cout << " -> ";
        }
    }

    std::cout << "\nDistance: " << std::fixed << std::setprecision(3)
              << route.distance << " miles"
              << "\nVisited nodes: " << route.visitedNodes << "\n";
}

//Resolve comma-separated building names into blocked graph vertices
std::unordered_set<int> parseBlocked(const CampusMap& campus, const std::string& input) {
    std::unordered_set<int> blocked;
    std::stringstream stream(input);
    std::string token;

    while (std::getline(stream, token, ',')) {
        if (token.empty()) {
            continue;
        }

        const auto place = campus.findPlace(token);
        if (place) {
            blocked.insert(*place);
        }
    }

    return blocked;
}

template <typename Function>
auto timed(Function&& fn) {
    const auto start = std::chrono::high_resolution_clock::now();
    auto result = fn();
    const auto end = std::chrono::high_resolution_clock::now();
    const auto micros =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return std::pair{result, micros};
}

void routeMode(const CampusMap& campus) {
    std::string startQuery;
    std::string destinationQuery;

    std::cout << "\nStart building: ";
    std::getline(std::cin, startQuery);
    std::cout << "Destination building: ";
    std::getline(std::cin, destinationQuery);

    const auto start = campus.findPlace(startQuery);
    const auto destination = campus.findPlace(destinationQuery);

    if (!start || !destination) {
        std::cout << "Could not resolve one or both building names.\n";
        return;
    }

    std::string blockedInput;
    std::cout << "Temporarily blocked buildings (comma-separated, Enter for none): ";
    std::getline(std::cin, blockedInput);
    auto blocked = parseBlocked(campus, blockedInput);
    blocked.erase(*start);
    blocked.erase(*destination);

    //Run both algorithms so route quality and search effort can be compared
    const auto [dijkstraResult, dijkstraMicros] = timed([&] {
        return dijkstra(campus.graph(), *start, *destination, blocked);
    });

    const auto [aStarResult, aStarMicros] = timed([&] {
        return aStar(
            campus.graph(),
            *start,
            *destination,
            [&campus](int from, int to) {
                return campus.heuristicMiles(from, to);
            },
            blocked
        );
    });

    std::cout << "\nA* route\n";
    std::cout << "--------\n";
    printPath(campus, aStarResult);

    std::cout << "\nAlgorithm comparison\n";
    std::cout << "--------------------\n";
    std::cout << "Dijkstra visited: " << dijkstraResult.visitedNodes
              << ", runtime: " << dijkstraMicros << " us\n";
    std::cout << "A* visited:       " << aStarResult.visitedNodes
              << ", runtime: " << aStarMicros << " us\n";

    if (dijkstraResult.found() && aStarResult.found()) {
        std::cout << "Shortest-path agreement: "
                  << (std::abs(dijkstraResult.distance - aStarResult.distance) < 1e-9
                          ? "yes"
                          : "no")
                  << "\n";
    }
}

void meetingMode(const CampusMap& campus) {
    std::string firstQuery;
    std::string secondQuery;

    std::cout << "\nPerson A building: ";
    std::getline(std::cin, firstQuery);
    std::cout << "Person B building: ";
    std::getline(std::cin, secondQuery);

    const auto first = campus.findPlace(firstQuery);
    const auto second = campus.findPlace(secondQuery);

    if (!first || !second) {
        std::cout << "Could not resolve one or both building names.\n";
        return;
    }

    const auto result = findFairMeetingPoint(campus, *first, *second);

    if (result.placeId < 0) {
        std::cout << "No mutually reachable meeting point was found.\n";
        return;
    }

    std::cout << "\nRecommended meeting point: "
              << campus.place(result.placeId).name << "\n";
    std::cout << "Objective: minimize the longer person's route, "
                 "then minimize total distance.\n";

    std::cout << "\nPerson A\n";
    printPath(campus, result.routeA);

    std::cout << "\nPerson B\n";
    printPath(campus, result.routeB);

    std::cout << "\nWorst individual distance: " << std::fixed
              << std::setprecision(3) << result.worstDistance << " miles\n";
    std::cout << "Combined distance: " << result.totalDistance << " miles\n";
}

}

int main(int argc, char* argv[]) {
    try {
        const std::string dataDirectory = argc > 1 ? argv[1] : "data";

        CampusMap campus;
        campus.loadPlaces(dataDirectory + "/places.csv");
        campus.loadEdges(dataDirectory + "/edges.csv");

        std::cout << "Geospatial Route Planner\n";
        std::cout << "========================\n";
        std::cout << "Loaded " << campus.graph().vertexCount()
                  << " places and " << campus.graph().edgeCount()
                  << " directed edges.\n";

        while (true) {
            std::cout << "\n1) Find shortest route\n"
                         "2) Find fair meeting point\n"
                         "3) List places\n"
                         "0) Exit\n"
                         "Choice: ";

            std::string choice;
            std::getline(std::cin, choice);

            if (choice == "0") {
                break;
            }
            if (choice == "1") {
                routeMode(campus);
            } else if (choice == "2") {
                meetingMode(campus);
            } else if (choice == "3") {
                std::cout << "\n";
                for (const auto& [id, place] : campus.places()) {
                    std::cout << std::setw(3) << id << "  "
                              << std::setw(5) << place.abbreviation << "  "
                              << place.name << "\n";
                }
            } else {
                std::cout << "Invalid choice.\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
