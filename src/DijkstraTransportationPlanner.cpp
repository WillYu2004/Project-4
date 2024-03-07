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
    }

};