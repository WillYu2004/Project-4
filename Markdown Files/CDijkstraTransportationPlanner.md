# Overview
`CDijkstraTransportationPlanner` is a concrete class derived from `CTransportationPlanner` that utilizes Dijkstra's algorithm to compute the shortest and fastest paths for various transportation modes.

## Constructor
- `CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config)`: Initializes the planner with the provided configuration.

## Destructor
- `~CDijkstraTransportationPlanner()`: Cleans up resources, ensuring proper deletion of the implementation details.

## Methods
- `std::size_t NodeCount() const noexcept`: Returns the total number of nodes in the map.
- `std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept`: Retrieves a node by its index in a sorted manner.
- `double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path)`: Computes the shortest path between two nodes.
- `double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path)`: Determines the fastest path using various transportation modes.
- `bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const`: Generates a description of the planned route.

## Sample Usage
The `CDijkstraTransportationPlanner` can be directly instantiated with a configuration object and used to calculate routes:

```cpp
std::shared_ptr<CTransportationPlanner::SConfiguration> config = std::make_shared<MyConfiguration>();
CDijkstraTransportationPlanner planner(config);

std::vector<CTransportationPlanner::TTripStep> path;
double duration = planner.FindFastestPath(startNode, endNode, path);

std::vector<std::string> description;
if (planner.GetPathDescription(path, description)) {
    for (const auto& step : description) {
        std::cout << step << std::endl;
    }
}
