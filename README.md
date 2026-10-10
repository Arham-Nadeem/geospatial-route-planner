# Geospatial Route Planner

A self-contained C++17 route-planning project that models a campus as a weighted geospatial graph and compares **Dijkstra's algorithm** with **A\*** for shortest-path routing.

The project also supports temporary route closures and a two-person meeting-point optimizer that minimizes the longer travel distance before minimizing total distance.

## Why This Project Is More Than a Basic Dijkstra Demo

This repository combines several computer-science concepts in one reproducible system:

- generic weighted adjacency-list graph
- Dijkstra shortest-path search
- A* search with an admissible Haversine-distance heuristic
- geospatial distance calculations
- CSV-driven map construction
- route reconstruction
- blocked-node constraints
- algorithm instrumentation
- automated correctness tests
- fairness-aware meeting-point optimization
- CMake build configuration

## Features

### Shortest-Path Routing

The user can select two buildings by name or abbreviation.

The application computes the route with both Dijkstra and A* and reports:

- route distance
- ordered path
- number of visited nodes
- runtime in microseconds
- whether both algorithms agree on the shortest distance

### A* Heuristic

A* uses straight-line Haversine distance between two coordinates as its heuristic.

Because the sample graph's edge weights are based on geographic distance multiplied by a factor of at least 1.0, the direct Haversine distance never overestimates the actual route cost. That makes the heuristic admissible for the included dataset.

### Temporary Closures

Buildings can be marked unavailable for a route. The search excludes blocked vertices while preserving the selected start and destination.

This demonstrates constrained graph traversal rather than only static shortest-path search.

### Fair Meeting-Point Optimization

Given two starting buildings, the application evaluates candidate destinations and chooses the one that:

1. minimizes the longer person's travel distance
2. uses total travel distance as a tie-breaker

This produces a more balanced meeting location than simply choosing a geographic midpoint.

## Project Structure

```text
geospatial-route-planner/
│
├── CMakeLists.txt
├── README.md
├── .gitignore
├── data/
│   ├── places.csv
│   └── edges.csv
├── include/
│   ├── campus.hpp
│   ├── geo.hpp
│   ├── graph.hpp
│   └── routing.hpp
├── src/
│   └── main.cpp
└── tests/
    └── route_tests.cpp
```

## Data Model

`places.csv` stores graph vertices:

```text
id,name,abbreviation,latitude,longitude
```

`edges.csv` stores walkable connections:

```text
from,to,multiplier
```

Each edge cost is:

```text
HaversineDistance(from, to) × multiplier
```

The multiplier represents the fact that walkable routes are usually longer than straight-line geographic distance.

The included dataset is synthetic and exists to make the repository fully reproducible without external APIs or proprietary map data.

## Algorithms

### Dijkstra

For nonnegative edge weights:

```text
Time:  O((V + E) log V)
Space: O(V)
```

using an adjacency list and binary-heap priority queue.

### A*

A* uses:

```text
f(n) = g(n) + h(n)
```

where:

- `g(n)` is the known route cost from the start
- `h(n)` is Haversine distance from the current node to the destination

With an informative admissible heuristic, A* can explore fewer nodes than Dijkstra while preserving shortest-path optimality.

## Build

### CMake

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/route_planner
```

On Windows with a multi-config generator, the executable may be under:

```text
build/Debug/route_planner.exe
```

## Tests

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The tests verify:

- shortest-path distance
- route reconstruction
- Dijkstra/A* agreement
- blocked-node behavior
- start-equals-destination behavior

## Example Workflow

```text
1) Find shortest route
2) Find fair meeting point
3) List places
0) Exit
```

Example route request:

```text
Start building: NRH
Destination building: BUS
Temporarily blocked buildings: LIB
```

The application will compute a route that avoids the blocked building and compare the search effort of Dijkstra and A*.

## Technical Improvements Over the Earlier Version

This version is designed as a standalone software-engineering portfolio project rather than a course-specific submission.

Key improvements include:

- removed course metadata and assignment scaffolding
- removed unavailable external course headers
- separated graph, routing, geospatial, and application concerns
- replaced O(V)-style neighbor representations with adjacency lists
- added A* alongside Dijkstra
- added Haversine geospatial heuristics
- added route constraints
- added automated tests
- added CMake
- added runtime and visited-node instrumentation
- added reproducible local map data
- added a more principled meeting-point objective

## Skills Demonstrated

- C++17
- graph data structures
- shortest-path algorithms
- priority queues
- templates
- geospatial computation
- algorithmic complexity
- heuristic search
- constrained routing
- software modularity
- automated testing
- CMake build systems
