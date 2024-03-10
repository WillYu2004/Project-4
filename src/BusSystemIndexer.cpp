#include "BusSystemIndexer.h"
#include <vector>
#include <algorithm>
#include <unordered_map>

struct CBusSystemIndexer::SImplementation{
    struct pair_hash{
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1,T2> &p) const{
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);

            return h1 ^ h2;
        }
    };

    std::shared_ptr<CBusSystem> DBusSystem;
    std::vector< std::shared_ptr<SStop> > DSortedStops;
    std::vector<std::shared_ptr<SRoute>> DSortedRoutes;
    std::unordered_map< TNodeID, std::shared_ptr<SStop> > DNodeIDToStop;
    std::unordered_map<std::pair<TNodeID, TNodeID>, std::unordered_set<std::shared_ptr<SRoute>>, pair_hash> DSrcDestToRoutes;

    static bool StopIDCompare(std::shared_ptr<SStop> left, std::shared_ptr<SStop> right) {
        return left->ID() < right->ID();
    }

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
        }
    }

    std::size_t StopCount() const noexcept{
        return DBusSystem->StopCount();
    }

    std::size_t RouteCount() const noexcept{
        return DBusSystem->RouteCount();
    }

    std::shared_ptr<SStop> SortedStopByIndex(std::size_t index) const noexcept{
        if (index < DSortedStops.size()) {
            return DSortedStops[index];
        }
        return nullptr;
    }

    std::shared_ptr<SRoute> SortedRouteByIndex(std::size_t index) const noexcept{
        if (index < DSortedRoutes.size()) {
            return DSortedRoutes[index];
        }
        return nullptr;
    }

    std::shared_ptr<SStop> StopByNodeID(TNodeID id) const noexcept{
        auto Search = DNodeIDToStop.find(id);
        if(Search != DNodeIDToStop.end()){
            return Search->second;
        }
        return nullptr;
    }

    bool RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<SRoute> > &routes) const noexcept{
        auto SearchKey = std::make_pair(src,dest);
        auto Search = DSrcDestToRoutes.find(SearchKey);
        if(Search != DSrcDestToRoutes.end()){
            routes = Search->second;
            return true;
        }
        return false;
    }

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
