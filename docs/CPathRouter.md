# CPathRouter Class Documentation
`CPathRouter` is an abstract class for pathfinding in a graph structure, designed to find the shortest path between vertices.
## Constructor and Destructor

- **Constructor**: Not applicable as `CPathRouter` is an abstract class.
- **Destructor**: Virtual, ensuring derived class destructors are called correctly.

## Public Member Types
- `TVertexID`: Represents a vertex identifier.
- `InvalidVertexID`: Represents an invalid vertex ID.
- `NoPathExists`: Represents the absence of a path between two vertices.

## Methods
- `std::size_t VertexCount() const noexcept`: Returns the total number of vertices.
- `TVertexID AddVertex(std::any tag) noexcept`: Adds a vertex with a specified tag.
- `std::any GetVertexTag(TVertexID id) const noexcept`: Retrieves the tag associated with a vertex.
- `bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept`: Adds an edge between two vertices.
- `bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept`: Precomputes paths to optimize pathfinding.
- `double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept`: Finds the shortest path between two vertices.

## Sample Usage
```cpp
// Assuming DerivedPathRouter is a concrete implementation of CPathRouter
DerivedPathRouter router;
auto vertexA = router.AddVertex("Start");
auto vertexB = router.AddVertex("End");
router.AddEdge(vertexA, vertexB, 1.0);

std::vector<CPathRouter::TVertexID> path;
double distance = router.FindShortestPath(vertexA, vertexB, path);

// Example 2: Finding the shortest path between two vertices
std::vector<CPathRouter::TVertexID> shortestPath;
double distance = router.FindShortestPath(vertex1, vertex3, shortestPath);

if (distance != CPathRouter::NoPathExists) {
    std::cout << "Shortest path distance: " << distance << std::endl;
    std::cout << "Path: ";
    for (auto vertex : shortestPath) {
        std::any tag = router.GetVertexTag(vertex);
        std::cout << std::any_cast<std::string>(tag) << " ";
    }
    std::cout << std::endl;
} else {
    std::cout << "No path exists between the selected vertices." << std::endl;
}

// Example 3: Precomputing paths for optimization (if supported by the implementation)
if (router.Precompute(std::chrono::steady_clock::now() + std::chrono::seconds(30))) {
    std::cout << "Precomputation completed within the deadline." << std::endl;
} else {
    std::cout << "Precomputation did not complete within the deadline." << std::endl;
}

// Example 4: Handling invalid vertices
CPathRouter::TVertexID invalidVertex = CPathRouter::InvalidVertexID;
auto pathDistance = router.FindShortestPath(vertex1, invalidVertex, shortestPath);
if (pathDistance == CPathRouter::NoPathExists) {
    std::cout << "Attempted to find a path to an invalid vertex." << std::endl;
}
