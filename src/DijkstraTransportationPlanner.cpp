#include "DijkstraTransportationPlanner.h"
#include "DijkstraPathRouter.h"
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
        
    }
    bool GetPathDescription(const std::vector< TTripStep > &path, std::vector< std::string > &desc) const {
        
    }
};