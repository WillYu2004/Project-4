# Overview
`CTransportationPlanner` is an abstract class designed to plan transportation routes using various modes of transport, such as walking, biking, and bus rides. It leverages `CStreetMap` and `CBusSystem` to calculate routes.

## Transportation Modes
- `ETransportationMode`: Enum specifying the mode of transportation (Walk, Bike, Bus).

## Configuration Structure
- `SConfiguration`: Holds configuration settings like maps, speed limits, and bus stop times.

## Destructor
- `~CTransportationPlanner()`: Virtual destructor for cleanup.

## Methods
- `std::size_t NodeCount() const noexcept`: Returns the number of nodes in the map.
- `std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept`: Retrieves a node by its index in a sorted list.
- `double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path)`: Calculates the shortest distance path.
- `double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path)`: Calculates the fastest time path considering different transportation modes.
- `bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const`: Generates a human-readable description of the route.

## Sample Usage
To use `CTransportationPlanner`, you must extend it and implement its virtual methods. Here's a conceptual example of how it might be used once implemented:

```cpp
class MyTransportationPlanner : public CTransportationPlanner {
    // Implementations of virtual methods
};

MyTransportationPlanner planner(config);
std::vector<CTransportationPlanner::TTripStep> path;
double time = planner.FindFastestPath(startNode, endNode, path);

std::vector<std::string> description;
if (planner.GetPathDescription(path, description)) {
    for (const auto& step : description) {
        std::cout << step << std::endl;
    }
}
