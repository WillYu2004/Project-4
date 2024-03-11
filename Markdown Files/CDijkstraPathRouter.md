## Overview
`CDijkstraPathRouter` implements the `CPathRouter` interface, utilizing Dijkstra's algorithm to find the shortest path between nodes in a graph.

## Constructor and Destructor
- `CDijkstraPathRouter()`: Initializes the path router.
- `~CDijkstraPathRouter()`: Cleans up resources.

## Methods
- `std::size_t VertexCount() const noexcept`: Returns the count of vertices in the graph.
- `TVertexID AddVertex(std::any tag) noexcept`: Adds a vertex with an associated tag and returns its ID.
- `std::any GetVertexTag(TVertexID id) const noexcept`: Retrieves the tag associated with a vertex by its ID.
- `bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept`: Adds an edge between two vertices with an optional bidirectional flag.
- `bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept`: Optional precomputation step to optimize pathfinding, not utilized in Dijkstra's algorithm but required by the interface.
- `double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept`: Finds the shortest path between two vertices and returns the path's total weight.

## Sample Usage
```cpp
CDijkstraPathRouter router;
auto v1 = router.AddVertex("Start");
auto v2 = router.AddVertex("Middle");
auto v3 = router.AddVertex("End");

router.AddEdge(v1, v2, 5.0);
router.AddEdge(v2, v3, 3.0);

std::vector<CPathRouter::TVertexID> path;
double distance = router.FindShortestPath(v1, v3, path);

if (distance != CPathRouter::NoPathExists) {
    std::cout << "Distance: " << distance << std::endl;
    for (auto& v : path) {
        std::cout << std::any_cast<std::string>(router.GetVertexTag(v)) << " -> ";
    }
    std::cout << "End" << std::endl;
} else {
    std::cout << "No path found." << std::endl;
}
