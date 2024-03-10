#include "DijkstraTransportationPlanner.h"
#include "DijkstraPathRouter.h"
#include "GeographicUtils.h"
#include "CSVBusSystem.h"
#include "unordered_map"

struct CDijkstraTransportationPlanner::SImplementation{
    std::shared_ptr< CStreetMap > DStreetMap;
    std::shared_ptr< CBusSystem > DBusSystem;
    std::unordered_map< CStreetMap::TNodeID, CPathRouter::TVertexID > DNodeToVertexID;
    CDijkstraPathRouter DShortestPathRouter;
    CDijkstraPathRouter DFastestPathRouterBike;
    CDijkstraPathRouter DFastestPathRouterWalkBus;

    SImplementation(std::shared_ptr<SConfiguration> config){
        DStreetMap = config->StreetMap();
        DBusSystem = config->BusSystem();
        double WalkSpeed = config->WalkSpeed();
        double BikeSpeed = config->BikeSpeed();
        double DefaultSpeedLimit = config->DefaultSpeedLimit();
        double BusStopTime = config->BusStopTime();
        int PrecomputeTime = config->PrecomputeTime();
        

        for(size_t Index = 0; Index < DStreetMap->NodeCount(); Index++){
            auto Node = DStreetMap->NodeByIndex(Index);
            auto VertexID = DShortestPathRouter.AddVertex(Node->ID());
            DFastestPathRouterBike.AddVertex(Node->ID());
            DFastestPathRouterWalkBus.AddVertex(Node->ID());
            DNodeToVertexID[Node->ID()] = VertexID;
        }
        for(size_t Index = 0; Index < DStreetMap->WayCount(); Index++){
            auto Way = DStreetMap->WayByIndex(Index);
            bool Bikable = Way->GetAttribute("bicycle") != "no";
            bool Bidirectional = Way->GetAttribute("oneway") != "yes";
            auto PreviousNodeID = Way->GetNodeID(0);
            for(size_t NodeIndex = 1; NodeIndex < Way->NodeCount(); NodeIndex++){
                auto NextNodeID = Way->GetNodeID(NodeIndex);

            }
        }
    }

    std::size_t NodeCount() const noexcept {
        return DStreetMap->NodeCount();
    }
    std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept {
        if(index >= NodeCount()){
            return nullptr;
        }
        // Assuming DStreetMap provides sorted access or implement sorting
        return DStreetMap->NodeByIndex(index);
    }

    double FindShortestPath(TNodeID src, TNodeID dest, std::vector< TNodeID > &path) {
        std::vector< CPathRouter::TVertexID > ShortestPath;
        auto SourcevertexID = DNodeToVertexID[src];
        auto DestinationvertexID = DNodeToVertexID[dest];
        auto Distance = DShortestPathRouter.FindShortestPath(SourcevertexID,DestinationvertexID,ShortestPath);
        path.clear();
        for(auto vertexID : ShortestPath){
            path.push_back(std::any_cast<TNodeID>(DShortestPathRouter.GetVertexTag(vertexID)));
        }
        return Distance;
    }
    double FindFastestPath(TNodeID src, TNodeID dest, std::vector< TTripStep > &path) {
    std::vector<CPathRouter::TVertexID> FastestPath;
    auto SourceVertexID = DNodeToVertexID[src];
    auto DestinationVertexID = DNodeToVertexID[dest];

    // Find the fastest path using bike
    auto BikeDuration = DFastestPathRouterBike.FindShortestPath(SourceVertexID, DestinationVertexID, FastestPath);
    if (BikeDuration != CPathRouter::NoPathExists) {
        path.clear();
        for (auto VertexID : FastestPath) {
            auto NodeID = std::any_cast<TNodeID>(DFastestPathRouterBike.GetVertexTag(VertexID));
            path.emplace_back(NodeID, ETransportationMode::Bike);
        }
        return BikeDuration;
    }

    // Find the fastest path using walk and bus
    std::vector<CPathRouter::TVertexID> WalkBusPath;
    auto WalkBusDuration = DFastestPathRouterWalkBus.FindShortestPath(SourceVertexID, DestinationVertexID, WalkBusPath);
    std::vector<CPathRouter::TVertexID> FastestPath;
    auto SourceVertexID = DNodeToVertexID[src];
    auto DestinationVertexID = DNodeToVertexID[dest];

    // Find the fastest path using bike
    auto BikeDuration = DFastestPathRouterBike.FindShortestPath(SourceVertexID, DestinationVertexID, FastestPath);
    if (BikeDuration != CPathRouter::NoPathExists) {
        path.clear();
        for (auto VertexID : FastestPath) {
            auto NodeID = std::any_cast<TNodeID>(DFastestPathRouterBike.GetVertexTag(VertexID));
            path.emplace_back(NodeID, ETransportationMode::Bike);
        }
        return BikeDuration;
    }

    // Find the fastest path using walk and bus
    std::vector<CPathRouter::TVertexID> WalkBusPath;
    auto WalkBusDuration = DFastestPathRouterWalkBus.FindShortestPath(SourceVertexID, DestinationVertexID, WalkBusPath);
    if (WalkBusDuration != CPathRouter::NoPathExists) {
        path.clear();
        ETransportationMode PrevMode = ETransportationMode::Walk;
        for (auto VertexID : WalkBusPath) {
            auto NodeID = std::any_cast<TNodeID>(DFastestPathRouterWalkBus.GetVertexTag(VertexID));
            auto Mode = ETransportationMode::Walk;
            
            // Check if the current node is a bus stop
            if (DBusSystem->StopByID(NodeID) != nullptr) {
                Mode = ETransportationMode::Bus;
            }
            
            // Add the node and mode to the path only if the mode has changed
            if (Mode != PrevMode) {
                path.emplace_back(NodeID, Mode);
                PrevMode = Mode;
            }
        }
        return WalkBusDuration;
    }

    return CPathRouter::NoPathExists;
    }

    bool GetPathDescription(const std::vector< TTripStep > &path, std::vector< std::string > &desc) const {
    }
};
