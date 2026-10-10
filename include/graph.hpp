\
#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename Vertex, typename Weight>
class WeightedGraph {
public:
    using Neighbor = std::pair<Vertex, Weight>;

    bool addVertex(const Vertex& vertex) {
        return adjacency_.emplace(vertex, std::vector<Neighbor>{}).second;
    }

    //Insert or update a directed weighted edge
    void addDirectedEdge(const Vertex& from, const Vertex& to, const Weight& weight) {
        if (!contains(from) || !contains(to)) {
            throw std::invalid_argument("Both edge endpoints must exist in the graph.");
        }

        auto& neighbors = adjacency_.at(from);
        for (auto& [neighbor, existingWeight] : neighbors) {
            if (neighbor == to) {
                existingWeight = weight;
                return;
            }
        }
        neighbors.emplace_back(to, weight);
        ++edgeCount_;
    }

    void addUndirectedEdge(const Vertex& a, const Vertex& b, const Weight& weight) {
        addDirectedEdge(a, b, weight);
        addDirectedEdge(b, a, weight);
    }

    bool contains(const Vertex& vertex) const {
        return adjacency_.find(vertex) != adjacency_.end();
    }

    const std::vector<Neighbor>& neighbors(const Vertex& vertex) const {
        static const std::vector<Neighbor> empty;
        const auto it = adjacency_.find(vertex);
        return it == adjacency_.end() ? empty : it->second;
    }

    //Return an edge weight only when the requested edge exists
    std::optional<Weight> weight(const Vertex& from, const Vertex& to) const {
        const auto it = adjacency_.find(from);
        if (it == adjacency_.end()) {
            return std::nullopt;
        }

        for (const auto& [neighbor, edgeWeight] : it->second) {
            if (neighbor == to) {
                return edgeWeight;
            }
        }
        return std::nullopt;
    }

    std::vector<Vertex> vertices() const {
        std::vector<Vertex> result;
        result.reserve(adjacency_.size());
        for (const auto& [vertex, _] : adjacency_) {
            result.push_back(vertex);
        }
        return result;
    }

    std::size_t vertexCount() const {
        return adjacency_.size();
    }

    std::size_t edgeCount() const {
        return edgeCount_;
    }

private:
    std::unordered_map<Vertex, std::vector<Neighbor>> adjacency_;
    std::size_t edgeCount_{0};
};
