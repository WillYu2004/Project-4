#include "BusSystemIndexer.h"
#include <vector>
#include <algorithm>
#include <unordered_map>

struct CBusSystemIndexer::SImplementation{
    //A custom hash function for pairs of TNodeIDs, enabling their use as keys in an unordered_map
    struct pair_hash{
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1,T2> &p) const{
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);

            return h1 ^ h2; // Combines the two hash values
        }
    };

    std::shared_ptr<CBusSystem> DBusSystem; // Holds the bus system to be indexed.
    std::vector< std::shared_ptr<SStop> > DSortedStops; // Sorted list of stops by ID.
    std::vector<std::shared_ptr<SRoute>> DSortedRoutes; // Sorted list of routes.
    std::unordered_map< TNodeID, std::shared_ptr<SStop> > DNodeIDToStop; // Maps node IDs to stops.
    std::unordered_map<std::pair<TNodeID, TNodeID>, std::unordered_set<std::shared_ptr<SRoute>>, pair_hash> DSrcDestToRoutes; // Maps pairs of source and destination node IDs to routes, using the custom hash function.

    // Compares two stops based on their IDs for sorting.
    static bool StopIDCompare(std::shared_ptr<SStop> left, std::shared_ptr<SStop> right) {
        return left->ID() < right->ID();
    }

    // Constructor initializes the indexer with a given bus system
    SImplementation(std::shared_ptr<CBusSystem> bussystem){
        DBusSystem = bussystem;
        for(size_t Index = 0; Index < DBusSystem->StopCount(); Index++){
            auto CurrentStop = DBusSystem->StopByIndex(Index);
            DSortedStops.push_back(CurrentStop);
            DNodeIDToStop[CurrentStop->NodeID()] = CurrentStop;
        }
        std::sort(DSortedStops.begin(),DSortedStops.end(),StopIDCompare);
        for(size_t Index = 0; Index < DBusSystem->RouteCount(); Index++){
            auto CurrentRoute = DBusSystem->RouteByIndex(Index);
            for(size_t StopIndex = 1; StopIndex < CurrentRoute->StopCount(); StopIndex++){
                auto SourceNodeID = DBusSystem->StopByID(CurrentRoute->GetStopID(StopIndex-1))->NodeID();
                auto DestinationNodeID = DBusSystem->StopByID(CurrentRoute->GetStopID(StopIndex))->NodeID();
                auto SearchKey = std::make_pair(SourceNodeID,DestinationNodeID);
                auto Search = DSrcDestToRoutes.find(SearchKey);
                if(Search != DSrcDestToRoutes.end()){
                    Search->second.insert(CurrentRoute);
                }
                else{
                    DSrcDestToRoutes[SearchKey] = {CurrentRoute};
                }
            }
            DSortedRoutes.push_back(CurrentRoute);
        }
    }

    // Returns the number of stops in the indexed bus system.
    std::size_t StopCount() const noexcept{
        return DBusSystem->StopCount();
    }

    // Returns the number of routes in the indexed bus system.
    std::size_t RouteCount() const noexcept{
        return DBusSystem->RouteCount();
    }

    // Returns a stop by its sorted index, facilitating ordered access.
    std::shared_ptr<SStop> SortedStopByIndex(std::size_t index) const noexcept{
        if (index < DSortedStops.size()) {
            return DSortedStops[index];
        }
        return nullptr;
    }

    // Returns a route by its sorted index
    std::shared_ptr<SRoute> SortedRouteByIndex(std::size_t index) const noexcept{
        if (index < DSortedRoutes.size()) {
            return DSortedRoutes[index];
        }
        return nullptr;
    }

    // Finds a stop by its node ID, using the precomputed mapping for efficiency
    std::shared_ptr<SStop> StopByNodeID(TNodeID id) const noexcept{
        auto Search = DNodeIDToStop.find(id);
        if(Search != DNodeIDToStop.end()){
            return Search->second;
        }
        return nullptr;
    }

    // Determines routes between two nodes, using precomputed mappings for efficiency.
    bool RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<SRoute> > &routes) const noexcept{
        auto SearchKey = std::make_pair(src,dest);
        auto Search = DSrcDestToRoutes.find(SearchKey);
        if(Search != DSrcDestToRoutes.end()){
            routes = Search->second;
            return true;
        }
        return false;
    }

    // Checks if there is any route between two nodes.
    bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept{
        auto SearchKey = std::make_pair(src,dest);
        auto Search = DSrcDestToRoutes.find(SearchKey);
        return Search != DSrcDestToRoutes.end();
    }

};

CBusSystemIndexer::CBusSystemIndexer(std::shared_ptr<CBusSystem> bussystem){
    DImplementation = std::make_unique<SImplementation>(bussystem);
}

CBusSystemIndexer::~CBusSystemIndexer(){

}

// Public member functions of CBusSystemIndexer that delegate to the corresponding SImplementation methods.
std::size_t CBusSystemIndexer::StopCount() const noexcept{
    return DImplementation->StopCount();
}

std::size_t CBusSystemIndexer::RouteCount() const noexcept{
    return DImplementation->RouteCount();
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SortedStopByIndex(std::size_t index) const noexcept{
    return DImplementation->SortedStopByIndex(index);
}

std::shared_ptr<CBusSystem::SRoute> CBusSystemIndexer::SortedRouteByIndex(std::size_t index) const noexcept{
    return DImplementation->SortedRouteByIndex(index);
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::StopByNodeID(TNodeID id) const noexcept{
    return DImplementation->StopByNodeID(id);
}

bool CBusSystemIndexer::RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<SRoute> > &routes) const noexcept{
    return DImplementation->RoutesByNodeIDs(src,dest,routes);
}

bool CBusSystemIndexer::RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept{
    return DImplementation->RouteBetweenNodeIDs(src,dest);
}
