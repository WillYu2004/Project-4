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
                double TimeWalk = Distance * 1609.34 / WalkSpeed; // Time in seconds assuming WalkSpeed is in m/s
                double TimeBus = Distance / BusSpeed * 3600; // Time in seconds assuming BusSpeed is in mph

                // Add edges for shortest path calculation
                DShortestPathRouter.AddEdge(PreviousVertexID, NextVertexID, Distance, Bidirectional);

                // Add edges for fastest bike path calculation, if bikable
                if (Bikable && TimeBike != CPathRouter::NoPathExists) {
                    DFastestPathRouterBike.AddEdge(PreviousVertexID, NextVertexID, TimeBike, Bidirectional);
                }

                // Add edges for fastest walk/bus path calculation
                double TimeWalkBus = TimeWalk; // Initial walking time
if (DBusSystem->StopByID(PreviousNodeID) != nullptr && DBusSystem->StopByID(NextNodeID) != nullptr) {
    // Check if there is a direct bus route between the two stops
    for (size_t RouteIndex = 0; RouteIndex < DBusSystem->RouteCount(); ++RouteIndex) {
        auto Route = DBusSystem->RouteByIndex(RouteIndex);
        bool PrevStopFound = false;
        bool NextStopFound = false;
        for (size_t StopIndex = 0; StopIndex < Route->StopCount(); ++StopIndex) {
            auto StopID = Route->GetStopID(StopIndex);
            if (StopID == PreviousNodeID) {
                PrevStopFound = true;
            } else if (StopID == NextNodeID) {
                NextStopFound = true;
            }
            if (PrevStopFound && NextStopFound) {
                // Calculate the bus travel time between the two stops
                double BusDistance = 0.0;
                for (size_t i = 0; i < Route->StopCount() - 1; ++i) {
                    auto Stop1 = DBusSystem->StopByID(Route->GetStopID(i));
                    auto Stop2 = DBusSystem->StopByID(Route->GetStopID(i + 1));

                    // Assuming we can get corresponding street map nodes for these bus stops
                    auto Node1 = DStreetMap->NodeByID(Stop1->NodeID()); // Assuming Stop objects have a NodeID() method
                    auto Node2 = DStreetMap->NodeByID(Stop2->NodeID());

                    if (Node1 && Node2) {
                        double SegmentDistance = SGeographicUtils::HaversineDistanceInMiles(Node1->Location(), Node2->Location());
                        BusDistance += SegmentDistance;
                    }
                }

                double BusSpeed = config->DefaultSpeedLimit();
                double TimeBus = (BusDistance / BusSpeed * 3600) + (Route->StopCount() - 1) * config->BusStopTime(); // Time in seconds
                TimeWalkBus = std::min(TimeWalkBus, TimeBus + 2 * config->BusStopTime()); // Account for walking time to/from bus stops
                break;
            }
        }
    }
}
DFastestPathRouterWalkBus.AddEdge(PreviousVertexID, NextVertexID, TimeWalkBus, Bidirectional);
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
            return BikeDuration / 3600.0;
        }

        // Find the fastest path using walk and bus
        std::vector<CPathRouter::TVertexID> WalkBusPath;
        auto WalkBusDuration = DFastestPathRouterWalkBus.FindShortestPath(SourceVertexID, DestinationVertexID, WalkBusPath);
        if (WalkBusDuration != CPathRouter::NoPathExists) {
            path.clear();
            ETransportationMode PrevMode = ETransportationMode::Walk;
            for (size_t i = 0; i < WalkBusPath.size(); ++i) {
                auto VertexID = WalkBusPath[i];
                auto NodeID = std::any_cast<TNodeID>(DFastestPathRouterWalkBus.GetVertexTag(VertexID));
                auto Mode = ETransportationMode::Walk;

                // Check if the current node is a bus stop
                if (DBusSystem->StopByID(NodeID) != nullptr) {
                    // Check if the next node is also a bus stop and there is a direct bus route between them
                    if (i < WalkBusPath.size() - 1) {
                        auto NextVertexID = WalkBusPath[i + 1];
                        auto NextNodeID = std::any_cast<TNodeID>(DFastestPathRouterWalkBus.GetVertexTag(NextVertexID));
                        if (DBusSystem->StopByID(NextNodeID) != nullptr) {
                            for (size_t RouteIndex = 0; RouteIndex < DBusSystem->RouteCount(); ++RouteIndex) {
                                auto Route = DBusSystem->RouteByIndex(RouteIndex);
                                bool PrevStopFound = false;
                                bool NextStopFound = false;
                                for (size_t StopIndex = 0; StopIndex < Route->StopCount(); ++StopIndex) {
                                    auto StopID = Route->GetStopID(StopIndex);
                                    if (StopID == NodeID) {
                                        PrevStopFound = true;
                                    } else if (StopID == NextNodeID) {
                                        NextStopFound = true;
                                    }
                                    if (PrevStopFound && NextStopFound) {
                                        Mode = ETransportationMode::Bus;
                                        break;
                                    }
                                }
                                if (Mode == ETransportationMode::Bus) {
                                    break;
                                }
                            }
                        }
                    }
                }

                // Add the node and mode to the path only if the mode has changed
                if (Mode != PrevMode) {
                    path.emplace_back(Mode, NodeID);
                    PrevMode = Mode;
                }
            }
            return WalkBusDuration / 3600.0; // Convert seconds to hours
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