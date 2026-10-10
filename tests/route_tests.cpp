\
#include <cassert>
#include <cmath>
#include <iostream>
#include <unordered_set>

#include "graph.hpp"
#include "routing.hpp"

int main() {
    WeightedGraph<int, double> graph;
    for (int i = 1; i <= 6; ++i) {
        graph.addVertex(i);
    }

    graph.addUndirectedEdge(1, 2, 1.0);
    graph.addUndirectedEdge(2, 3, 1.0);
    graph.addUndirectedEdge(1, 4, 2.0);
    graph.addUndirectedEdge(4, 5, 1.0);
    graph.addUndirectedEdge(5, 3, 0.5);
    graph.addUndirectedEdge(3, 6, 1.0);

    //Verify shortest-path distance and endpoint reconstruction
    const auto shortest = dijkstra(graph, 1, 6);
    assert(shortest.found());
    assert(std::abs(shortest.distance - 3.0) < 1e-9);
    assert(shortest.path.front() == 1);
    assert(shortest.path.back() == 6);

    const auto heuristic = [](int, int) { return 0.0; };
    const auto aStarResult = aStar(graph, 1, 6, heuristic);
    assert(aStarResult.found());
    assert(std::abs(aStarResult.distance - shortest.distance) < 1e-9);

    //Verify routing fails when the only path to the goal is blocked
    std::unordered_set<int> blocked{3};
    const auto blockedRoute = dijkstra(graph, 1, 6, blocked);
    assert(!blockedRoute.found());

    const auto sameNode = dijkstra(graph, 2, 2);
    assert(sameNode.found());
    assert(sameNode.distance == 0.0);
    assert(sameNode.path.size() == 1);

    std::cout << "All route tests passed.\n";
    return 0;
}
