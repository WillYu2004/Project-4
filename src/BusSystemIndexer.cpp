#include "BusSystemIndexer.h"
#include "CSVBusSystem.h"
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <vector>

class CBusSystemIndexer {
private:
    struct SImplementation {
        std::shared_ptr<CCSVBusSystem> BusSystem;
        std::vector<std::shared_ptr<CCSVBusSystem::SStop>> SortedStops;
        std::vector<std::shared_ptr<CCSVBusSystem::SRoute>> SortedRoutes;
        std::unordered_map<CCSVBusSystem::TStopID, std::shared_ptr<CCSVBusSystem::SStop>> StopIDtoStopMap;

        SImplementation(std::shared_ptr<CCSVBusSystem> busSystem) : BusSystem(std::move(busSystem)) {
            IndexStops();
            IndexRoutes();
        }

        void IndexStops() {
            for (std::size_t i = 0, n = BusSystem->StopCount(); i < n; ++i) {
                auto stop = BusSystem->StopByIndex(i);
                SortedStops.push_back(stop);
                StopIDtoStopMap[stop->ID()] = stop;
            }

            std::sort(SortedStops.begin(), SortedStops.end(),
                      [](const std::shared_ptr<CCSVBusSystem::SStop>& a, const std::shared_ptr<CCSVBusSystem::SStop>& b) {
                          return a->ID() < b->ID();
                      });
        }

        void IndexRoutes() {
            for (std::size_t i = 0, n = BusSystem->RouteCount(); i < n; ++i) {
                auto route = BusSystem->RouteByIndex(i);
                SortedRoutes.push_back(route);
            }

            std::sort(SortedRoutes.begin(), SortedRoutes.end(),
                      [](const std::shared_ptr<CCSVBusSystem::SRoute>& a, const std::shared_ptr<CCSVBusSystem::SRoute>& b) {
                          return a->Name() < b->Name();
                      });
        }
    };

    std::unique_ptr<SImplementation> DImplementation;

public:
    using TStopID = CCSVBusSystem::TStopID;
    using TNodeID = CStreetMap::TNodeID;

    CBusSystemIndexer(std::shared_ptr<CCSVBusSystem> busSystem)
    : DImplementation(std::make_unique<SImplementation>(std::move(busSystem))) {}

    ~CBusSystemIndexer() = default;

    std::size_t StopCount() const noexcept {
        return DImplementation->SortedStops.size();
    }

    std::size_t RouteCount() const noexcept {
        return DImplementation->SortedRoutes.size();
    }

    std::shared_ptr<CCSVBusSystem::SStop> SortedStopByIndex(std::size_t index) const noexcept {
        if (index >= StopCount()) {
            return nullptr;
        }
        return DImplementation->SortedStops[index];
    }

    std::shared_ptr<CCSVBusSystem::SRoute> SortedRouteByIndex(std::size_t index) const noexcept {
        if (index >= RouteCount()) {
            return nullptr;
        }
        return DImplementation->SortedRoutes[index];
    }

    std::shared_ptr<CCSVBusSystem::SStop> StopByID(TStopID id) const noexcept {
        auto it = DImplementation->StopIDtoStopMap.find(id);
        if (it != DImplementation->StopIDtoStopMap.end()) {
            return it->second;
        }
        return nullptr;
    }
    bool RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<CCSVBusSystem::SRoute>>& routes) const noexcept {
        bool found = false;
        auto srcStop = StopByID(src);
        auto destStop = StopByID(dest);
        if (!srcStop || !destStop) {
            return false;
        }

        TStopID srcStopID = srcStop->ID();
        TStopID destStopID = destStop->ID();

        for (const auto& route : DImplementation->SortedRoutes) {
            const auto& stopIDs = route->StopIDs;
            auto srcIt = std::find(stopIDs.begin(), stopIDs.end(), srcStopID);
            auto destIt = std::find(stopIDs.begin(), stopIDs.end(), destStopID);
            if (srcIt != stopIDs.end() && destIt != stopIDs.end() && srcIt < destIt) {
                routes.insert(route);
                found = true;
            }
        }

        return found;
    }

    bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept {
        std::unordered_set<std::shared_ptr<CCSVBusSystem::SRoute>> routes;
        return RoutesByNodeIDs(src, dest, routes);
    }
};
