\
#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "graph.hpp"

template <typename Vertex>
struct RouteResult {
    std::vector<Vertex> path;
    double distance{std::numeric_limits<double>::infinity()};
    std::size_t visitedNodes{0};

    bool found() const {
        return !path.empty();
    }
};

template <typename Vertex>
//Rebuild the final path by following predecessor links backward
std::vector<Vertex> reconstructPath(
    const Vertex& start,
    const Vertex& goal,
    const std::unordered_map<Vertex, Vertex>& previous) {

    std::vector<Vertex> path;
    Vertex current = goal;
    path.push_back(current);

    while (current != start) {
        const auto it = previous.find(current);
        if (it == previous.end()) {
            return {};
        }
        current = it->second;
        path.push_back(current);
    }

    std::reverse(path.begin(), path.end());
    return path;
}

template <typename Vertex>
//Compute the shortest path using Dijkstra's algorithm
RouteResult<Vertex> dijkstra(
    const WeightedGraph<Vertex, double>& graph,
    const Vertex& start,
    const Vertex& goal,
    const std::unordered_set<Vertex>& blocked = {}) {

    RouteResult<Vertex> result;
    if (!graph.contains(start) || !graph.contains(goal)) {
        return result;
    }
    if ((blocked.count(start) || blocked.count(goal)) && start != goal) {
        return result;
    }
    if (start == goal) {
        result.path = {start};
        result.distance = 0.0;
        result.visitedNodes = 1;
        return result;
    }

    using QueueItem = std::pair<double, Vertex>;
    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> frontier;

    std::unordered_map<Vertex, double> distance;
    std::unordered_map<Vertex, Vertex> previous;

    distance[start] = 0.0;
    frontier.emplace(0.0, start);

    while (!frontier.empty()) {
        const auto [currentDistance, current] = frontier.top();
        frontier.pop();

        //Discard stale queue entries after a shorter path has already been found
        const auto known = distance.find(current);
        if (known == distance.end() || currentDistance > known->second) {
            continue;
        }

        ++result.visitedNodes;

        if (current == goal) {
            result.path = reconstructPath(start, goal, previous);
            result.distance = currentDistance;
            return result;
        }

        for (const auto& [neighbor, weight] : graph.neighbors(current)) {
            if (blocked.count(neighbor)) {
                continue;
            }

            //Relax the edge when this route improves the known distance
            const double candidate = currentDistance + weight;
            const auto neighborIt = distance.find(neighbor);

            if (neighborIt == distance.end() || candidate < neighborIt->second) {
                distance[neighbor] = candidate;
                previous[neighbor] = current;
                frontier.emplace(candidate, neighbor);
            }
        }
    }

    return result;
}

template <typename Vertex, typename Heuristic>
//Compute the shortest path using A* and a caller-provided heuristic
RouteResult<Vertex> aStar(
    const WeightedGraph<Vertex, double>& graph,
    const Vertex& start,
    const Vertex& goal,
    Heuristic heuristic,
    const std::unordered_set<Vertex>& blocked = {}) {

    RouteResult<Vertex> result;
    if (!graph.contains(start) || !graph.contains(goal)) {
        return result;
    }
    if ((blocked.count(start) || blocked.count(goal)) && start != goal) {
        return result;
    }
    if (start == goal) {
        result.path = {start};
        result.distance = 0.0;
        result.visitedNodes = 1;
        return result;
    }

    using QueueItem = std::pair<double, Vertex>;
    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> frontier;

    std::unordered_map<Vertex, double> gScore;
    std::unordered_map<Vertex, Vertex> previous;

    gScore[start] = 0.0;
    frontier.emplace(heuristic(start, goal), start);

    while (!frontier.empty()) {
        const auto [_, current] = frontier.top();
        frontier.pop();

        ++result.visitedNodes;

        if (current == goal) {
            result.path = reconstructPath(start, goal, previous);
            result.distance = gScore.at(goal);
            return result;
        }

        const double currentG = gScore.at(current);

        for (const auto& [neighbor, weight] : graph.neighbors(current)) {
            if (blocked.count(neighbor)) {
                continue;
            }

            //Update g-score and enqueue a new f-score when a better route is found
            const double candidate = currentG + weight;
            const auto neighborIt = gScore.find(neighbor);

            if (neighborIt == gScore.end() || candidate < neighborIt->second) {
                gScore[neighbor] = candidate;
                previous[neighbor] = current;
                const double fScore = candidate + heuristic(neighbor, goal);
                frontier.emplace(fScore, neighbor);
            }
        }
    }

    return result;
}
