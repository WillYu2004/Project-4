#include "DijkstraTransportationPlanner.h"
#include "DijkstraPathRouter.h"
#include "GeographicUtils.h"
#include "CSVBusSystem.h"
#include "unordered_map"
#include <sstream>
#include <iomanip>
#include <algorithm>

struct CDijkstraTransportationPlanner::SImplementation{
    std::shared_ptr< CStreetMap > DStreetMap;
    std::shared_ptr< CBusSystem > DBusSystem;
    std::unordered_map< CStreetMap::TNodeID, CPathRouter::TVertexID > DNodeToVertexID;
    CDijkstraPathRouter DShortestPathRouter;
    CDijkstraPathRouter DFastestPathRouterBike;
    CDijkstraPathRouter DFastestPathRouterWalkBus;
    std::vector<TNodeID> SortedNodeIDs;

    SImplementation(std::shared_ptr<SConfiguration> config) {
        if (config) {
            DStreetMap = config->StreetMap();
            DBusSystem = config->BusSystem();
            double WalkSpeed = config->WalkSpeed();
            double BikeSpeed = config->BikeSpeed();
            double DefaultSpeedLimit = config->DefaultSpeedLimit();
            double BusStopTime = config->BusStopTime();
            int PrecomputeTime = config->PrecomputeTime();
        }

        double BusSpeed = config ? config->DefaultSpeedLimit() : 0;
        double WalkSpeed = config ? config->WalkSpeed() : 0;
        double BikeSpeed = config ? config->BikeSpeed() : 0;

        for (size_t Index = 0; Index < DStreetMap->NodeCount(); ++Index) {
            auto Node = DStreetMap->NodeByIndex(Index);
            SortedNodeIDs.push_back(Node->ID());
        }

        // Sort the node IDs
        std::sort(SortedNodeIDs.begin(), SortedNodeIDs.end());

        for (size_t Index = 0; Index < DStreetMap->NodeCount(); Index++) {
            auto Node = DStreetMap->NodeByIndex(Index);
            auto VertexID = DShortestPathRouter.AddVertex(Node->ID());
            DFastestPathRouterBike.AddVertex(Node->ID());
            DFastestPathRouterWalkBus.AddVertex(Node->ID());
            DNodeToVertexID[Node->ID()] = VertexID;
        }

        for (size_t Index = 0; Index < DStreetMap->WayCount(); Index++) {
            auto Way = DStreetMap->WayByIndex(Index);
            bool Bikable = Way->GetAttribute("bicycle") != "no";
            bool Bidirectional = Way->GetAttribute("oneway") != "yes";
            auto PreviousNodeID = Way->GetNodeID(0);

            for (size_t NodeIndex = 1; NodeIndex < Way->NodeCount(); NodeIndex++) {
                auto PreviousNodeID = Way->GetNodeID(NodeIndex - 1);
                auto NextNodeID = Way->GetNodeID(NodeIndex);

                auto PreviousNode = DStreetMap->NodeByID(PreviousNodeID);
                auto NextNode = DStreetMap->NodeByID(NextNodeID);
                double Distance = SGeographicUtils::HaversineDistanceInMiles(PreviousNode->Location(), NextNode->Location());

                auto PreviousVertexID = DNodeToVertexID[PreviousNodeID];
                auto NextVertexID = DNodeToVertexID[NextNodeID];

                double TimeBike = Bikable ? (Distance / BikeSpeed * 3600) : CPathRouter::NoPathExists; // Time in seconds
                double TimeWalk = Distance / WalkSpeed * 3600; // Time in seconds assuming walk speed is in m/s
                double TimeBus = Distance / BusSpeed * 3600; // Time in seconds assuming bus speed is in m/s

                // Add edges for shortest path calculation
                DShortestPathRouter.AddEdge(PreviousVertexID, NextVertexID, Distance, Bidirectional);

                // Add edges for fastest bike path calculation, if bikable
                if (Bikable && TimeBike != CPathRouter::NoPathExists) {
                    DFastestPathRouterBike.AddEdge(PreviousVertexID, NextVertexID, TimeBike, Bidirectional);
                }

                // Add edges for fastest walk/bus path calculation
                DFastestPathRouterWalkBus.AddEdge(PreviousVertexID, NextVertexID, TimeBus, Bidirectional);
            }
        }
    }

    std::size_t NodeCount() const noexcept {
        return DStreetMap->NodeCount();
    }
    std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept {
        if (index >= NodeCount()) {
            return nullptr;
        }
        TNodeID nodeId = SortedNodeIDs[index];
        return DStreetMap->NodeByID(nodeId);
    }

    double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID>& path) {
    auto srcVertexIt = DNodeToVertexID.find(src);
    auto destVertexIt = DNodeToVertexID.find(dest);
    if (srcVertexIt == DNodeToVertexID.end() || destVertexIt == DNodeToVertexID.end()) {
        return CPathRouter::NoPathExists;
    }
    std::vector<CPathRouter::TVertexID> ShortestPath;
    auto SourceVertexID = srcVertexIt->second;
    auto DestinationVertexID = destVertexIt->second;
    auto Distance = DShortestPathRouter.FindShortestPath(SourceVertexID, DestinationVertexID, ShortestPath);
    if (Distance != CPathRouter::NoPathExists) {
        path.clear();
        for (auto vertexID : ShortestPath) {
            path.push_back(std::any_cast<TNodeID>(DShortestPathRouter.GetVertexTag(vertexID)));
        }
    }
    return Distance;
}

    double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep>& path) {
    std::vector<CPathRouter::TVertexID> FastestPath;
    auto SourceVertexID = DNodeToVertexID[src];
    auto DestinationVertexID = DNodeToVertexID[dest];

    // Find the fastest path using bike
    auto BikeDuration = DFastestPathRouterBike.FindShortestPath(SourceVertexID, DestinationVertexID, FastestPath);
    if (BikeDuration != CPathRouter::NoPathExists) {
        path.clear();
        for (auto VertexID : FastestPath) {
            auto NodeID = std::any_cast<TNodeID>(DFastestPathRouterBike.GetVertexTag(VertexID));
            path.emplace_back(CTransportationPlanner::ETransportationMode::Bike, NodeID);
        }
        return BikeDuration / 3600;
    }

    // Find the fastest path using walk and bus
    double minDuration = std::numeric_limits<double>::max();
    std::vector<CPathRouter::TVertexID> walkBusPath;

    for (size_t i = 0; i < DBusSystem->StopCount(); ++i) {
        auto busStop = DBusSystem->StopByIndex(i);
        if (!busStop) continue;

        auto busStopVertexID = DNodeToVertexID[busStop->ID()];
        std::vector<CPathRouter::TVertexID> walkPath;
        double walkToBusTime = DShortestPathRouter.FindShortestPath(SourceVertexID, busStopVertexID, walkPath);
        if (walkToBusTime == CPathRouter::NoPathExists) continue;

        for (size_t j = 0; j < DBusSystem->RouteCount(); ++j) {
            auto route = DBusSystem->RouteByIndex(j);
            if (!route) continue;

            bool containsSrcStop = false;
            bool containsDestStop = false;
            std::vector<TNodeID> busRoute;

            // Check if the bus route contains the source and destination stops
            for (size_t k = 0; k < route->StopCount(); ++k) {
                auto stopID = route->GetStopID(k);
                if (stopID == busStop->ID()) {
                    containsSrcStop = true;
                } else if (stopID == dest) {
                    containsDestStop = true;
                }

                // Reconstruct the bus route path if both source and destination stops are found
                if (containsSrcStop && containsDestStop) {
                    size_t srcIndex = std::numeric_limits<size_t>::max();
                    size_t destIndex = std::numeric_limits<size_t>::max();
                    for (size_t l = 0; l < route->StopCount(); ++l) {
                        if (route->GetStopID(l) == busStop->ID()) {
                            srcIndex = l;
                        } else if (route->GetStopID(l) == dest) {
                            destIndex = l;
                        }
                    }

                    if (srcIndex != std::numeric_limits<size_t>::max() && destIndex != std::numeric_limits<size_t>::max()) {
                        for (size_t m = srcIndex; m <= destIndex; ++m) {
                            busRoute.push_back(route->GetStopID(m));
                        }
                    }
                    break;
                }
            }

            if (!containsSrcStop || !containsDestStop) continue;

            // Find the shortest path from the bus stop to the destination
            double busTravelTime = DFastestPathRouterWalkBus.FindShortestPath(busStopVertexID, DNodeToVertexID[dest], FastestPath);

            double totalDuration = walkToBusTime + busTravelTime;
            if (totalDuration < minDuration) {
                minDuration = totalDuration;
                walkBusPath.clear();

                // Build the walk-bus path
                for (auto vertexID : walkPath) {
                    auto nodeID = std::any_cast<TNodeID>(DShortestPathRouter.GetVertexTag(vertexID));
                    walkBusPath.push_back(vertexID);
                }
                walkBusPath.push_back(busStopVertexID);

                for (auto nodeID : busRoute) {
                    auto vertexID = DNodeToVertexID[nodeID];
                    walkBusPath.push_back(vertexID);
                }
            }
        }
    }

    if (minDuration != std::numeric_limits<double>::max()) {
        path.clear();
        for (auto vertexID : walkBusPath) {
            auto nodeID = std::any_cast<TNodeID>(DFastestPathRouterWalkBus.GetVertexTag(vertexID));
            auto mode = (vertexID == walkBusPath.front() || vertexID == walkBusPath.back()) ? ETransportationMode::Walk : ETransportationMode::Bus;
            path.emplace_back(mode, nodeID);
        }
        return minDuration / 3600;
    }

    return CPathRouter::NoPathExists;
}

    bool GetPathDescription(const std::vector<TTripStep>& path, std::vector<std::string>& desc) const {
        if (path.empty()) {
            return false;
        }

        // Start location
        auto StartNode = DStreetMap->NodeByID(path.front().second);
        if (!StartNode) return false;

        auto StartLocation = StartNode->Location();
        std::stringstream StartSS;
        StartSS << "Start at " << SGeographicUtils::ConvertLLToDMS(StartLocation);
        desc.push_back(StartSS.str());

        for (size_t i = 0; i < path.size() - 1; ++i) {
            const auto& step = path[i];
            const auto& nextStep = path[i + 1];

            auto currentNode = DStreetMap->NodeByID(step.second);
            auto nextNode = DStreetMap->NodeByID(nextStep.second);
            if (!currentNode || !nextNode) return false;

            auto distance = SGeographicUtils::HaversineDistanceInMiles(currentNode->Location(), nextNode->Location());
            std::string modeDescription = step.first == ETransportationMode::Walk ? "Walk" : (step.first == ETransportationMode::Bike ? "Bike" : "Take Bus");

            std::stringstream segmentDesc;
            segmentDesc << modeDescription << " along " << std::fixed << std::setprecision(2) << distance << " miles";
            desc.push_back(segmentDesc.str());
        }

        // End location
        auto EndNode = DStreetMap->NodeByID(path.back().second);
        if (!EndNode) return false;

        auto EndLocation = EndNode->Location();
        std::stringstream EndSS;
        EndSS << "End at " << SGeographicUtils::ConvertLLToDMS(EndLocation);
        desc.push_back(EndSS.str());

        return true;
    }

};
// Constructor
CDijkstraTransportationPlanner::CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config)
    : DImplementation(std::make_unique<SImplementation>(config)) {}

// Destructor
CDijkstraTransportationPlanner::~CDijkstraTransportationPlanner() = default;

std::size_t CDijkstraTransportationPlanner::NodeCount() const noexcept {
    return DImplementation->NodeCount();
}

std::shared_ptr<CStreetMap::SNode> CDijkstraTransportationPlanner::SortedNodeByIndex(std::size_t index) const noexcept {
    return DImplementation->SortedNodeByIndex(index);
}

double CDijkstraTransportationPlanner::FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID>& path) {
    return DImplementation->FindShortestPath(src, dest, path);
}

double CDijkstraTransportationPlanner::FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep>& path) {
    return DImplementation->FindFastestPath(src, dest, path);
}

bool CDijkstraTransportationPlanner::GetPathDescription(const std::vector<TTripStep>& path, std::vector<std::string>& desc) const {
    return DImplementation->GetPathDescription(path, desc);
}