#include "DijkstraTransportationPlanner.h"
#include "DijkstraPathRouter.h"
#include "GeographicUtils.h"
#include "CSVBusSystem.h"
#include "unordered_map"
#include <sstream>
#include <iomanip>
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

bool GetPathDescription(const std::vector<TTripStep>& path, std::vector<std::string>& desc) const {
    desc.clear();

    if (path.empty()) {
        return false;
    }

    // Start location
    auto StartNode = DStreetMap->NodeByID(path.front().second);
    auto StartLocation = StartNode->Location();
    std::stringstream StartSS;
    StartSS << "Start at " << SGeographicUtils::ConvertLLToDMS(StartLocation);
    desc.push_back(StartSS.str());

    ETransportationMode PrevMode = path.front().first;
    TNodeID PrevNodeID = path.front().second;

    for (size_t i = 1; i < path.size(); ++i) {
        auto CurrentStep = path[i];
        auto CurrentNode = DStreetMap->NodeByID(CurrentStep.second);
        auto CurrentLocation = CurrentNode->Location();

        if (CurrentStep.first != PrevMode) {
            // Mode change
            if (CurrentStep.first == ETransportationMode::Bus) {
                auto PrevStop = DBusSystem->StopByID(PrevNodeID);
                auto CurrentStop = DBusSystem->StopByID(CurrentStep.second);
                std::stringstream BusSS;
                BusSS << "Take Bus from stop " << PrevStop->ID() << " to stop " << CurrentStop->ID();
                desc.push_back(BusSS.str());
            } else {
                auto PrevNode = DStreetMap->NodeByID(PrevNodeID);
                double Distance = SGeographicUtils::HaversineDistanceInMiles(PrevNode->Location(), CurrentLocation);
                double Bearing = SGeographicUtils::CalculateBearing(PrevNode->Location(), CurrentLocation);
                std::string Direction = SGeographicUtils::BearingToDirection(Bearing);
                std::stringstream WalkBikeSS;
                WalkBikeSS << (CurrentStep.first == ETransportationMode::Walk ? "Walk" : "Bike") << " "
                           << Direction << " for " << std::fixed << std::setprecision(1) << Distance << " mi";
                desc.push_back(WalkBikeSS.str());
            }
        } else {
            // Same mode
            double Distance = SGeographicUtils::HaversineDistanceInMiles(DStreetMap->NodeByID(PrevNodeID)->Location(),
                                                                         CurrentLocation);
            double Bearing = SGeographicUtils::CalculateBearing(DStreetMap->NodeByID(PrevNodeID)->Location(),
                                                                CurrentLocation);
            std::string Direction = SGeographicUtils::BearingToDirection(Bearing);
            std::stringstream SameModeSS;
            SameModeSS << (CurrentStep.first == ETransportationMode::Walk ? "Walk" : "Bike") << " "
                       << Direction << " for " << std::fixed << std::setprecision(1) << Distance << " mi";
            desc.push_back(SameModeSS.str());
        }

        PrevMode = CurrentStep.first;
        PrevNodeID = CurrentStep.second;
    }

    // End location
    auto EndNode = DStreetMap->NodeByID(path.back().second);
    auto EndLocation = EndNode->Location();
    std::stringstream EndSS;
    EndSS << "End at " << SGeographicUtils::ConvertLLToDMS(EndLocation);
    desc.push_back(EndSS.str());

    return true;
}

};
