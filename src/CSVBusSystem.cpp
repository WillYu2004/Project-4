#include "CSVBusSystem.h"
#include <vector>
#include <unordered_map>

// Implementation structure defined within CCSVBusSystem to encapsulate details
struct CCSVBusSystem::SImplementation {

    struct SStop : public CCSVBusSystem::SStop{
        
        SStop(TStopID sid, CStreetMap::TNodeID nid) : SID(sid), NID(nid){

        }

        ~SStop(){

        }
        //SStop Variables
        TStopID SID;
        CStreetMap::TNodeID NID;

        TStopID ID() const noexcept override {
            return SID;
        }
        
        CStreetMap::TNodeID NodeID() const noexcept override{
            return NID;
        }
    };

    struct SRoute : public CCSVBusSystem::SRoute{

        SRoute(){

        }

        ~SRoute(){

        }
        //SRoute Variables
        std::string RName;
        std::vector<TStopID> StopIDs;

        std::string Name() const noexcept override{
            return RName;
        }

        std::size_t StopCount() const noexcept override{
            return StopIDs.size();
        }

        TStopID GetStopID(std::size_t index) const noexcept override{
            if (index < StopIDs.size()) {
                return StopIDs[index];
            }
            return CBusSystem::InvalidStopID;
        }
    };

    // Maps and vectors to store stops and routes
    std::unordered_map<TStopID, std::shared_ptr<SStop>> StopsByID;
    std::vector<std::shared_ptr<SStop>> StopsByIndex;
    std::unordered_map<std::string, std::shared_ptr<SRoute>> RoutesByName; // Assuming route names are unique
    std::vector<std::shared_ptr<SRoute>> RoutesByIndex;

    SImplementation(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc) {
    std::vector<std::string> stopRow;
    bool isFirstRowStops = true; // To skip the header for stops
    
    // Corrected parsing logic for stops
    // Assuming isFirstRowStops is a boolean flag initialized to true before this loop
    while (stopsrc->ReadRow(stopRow)) {
    if (isFirstRowStops) {
        isFirstRowStops = false; // Skip the first row (headers)
        continue;
    }

    // Ensure that the row has at least 2 columns before proceeding
    if (stopRow.size() < 2) {
        continue; // Skip rows that don't have at least two columns
    }

    // Parse the stop ID and node ID from the row
    TStopID stopID = std::stoull(stopRow[0]);
    CStreetMap::TNodeID nodeID = std::stoull(stopRow[1]);

    // Create a new SStop object and store it
    auto stop = std::make_shared<SStop>(stopID, nodeID);
    StopsByID[stopID] = stop;
    StopsByIndex.push_back(stop);
}


    std::unordered_map<std::string, std::vector<TStopID>> routeStopsTemp;
    std::vector<std::string> routeRow;
    bool isFirstRowRoutes = true; // To skip the header for routes

// Parsing logic for routes
while (routesrc->ReadRow(routeRow)) {
    if (isFirstRowRoutes) {
        isFirstRowRoutes = false; // Skip the first row (header)
        continue;
    }

    // Proceed only if the row has at least two columns: route name and stop ID
    if (routeRow.size() < 2) {
        // If there are not at least two columns, skip this row
        continue;
    }

    std::string routeName = routeRow[0];
    TStopID stopID = std::stoull(routeRow[1]);

    // Add the stop ID to the vector of stop IDs for this route name
    routeStopsTemp[routeName].push_back(stopID);
}

// Construct routes from the temporary map
for (const auto& routePair : routeStopsTemp) {
    auto route = std::make_shared<SRoute>();
    route->RName = routePair.first; // The route name
    route->StopIDs = routePair.second; // Vector of stop IDs associated with this route

    // Store the newly created route in the appropriate data structures for later retrieval
    RoutesByName[route->RName] = route;
    RoutesByIndex.push_back(route);
}

}




    // Returns the number of stops in the system
    std::size_t StopCount() const noexcept{
        return StopsByIndex.size();
    }

    std::size_t RouteCount() const noexcept{
        return RoutesByIndex.size();
    }

    std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept{
        if(index < StopsByIndex.size()){
            return StopsByIndex[index];
        }
        return nullptr;
    }

    std::shared_ptr<SStop> StopByID(TStopID id) const noexcept{
        auto Search = StopsByID.find(id);
        if(StopsByID.end() != Search){
            return Search->second;
        }
        return nullptr;
    }

    std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept{
        if(index < RoutesByIndex.size()){
            return RoutesByIndex[index];
        }
        return nullptr;
    }

    std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept{
        auto Search = RoutesByName.find(name);
        if(RoutesByName.end() != Search){
            return Search->second;
        }
        return nullptr;
    }
};

// Constructor for the CCSVBusSystem
CCSVBusSystem::CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc)
: DImplementation(std::make_unique<SImplementation>(stopsrc, routesrc)) {}

// Destructor
CCSVBusSystem::~CCSVBusSystem(){};

// Returns the number of stops in the system
std::size_t CCSVBusSystem::StopCount() const noexcept {
    return DImplementation->StopCount();
}

// Returns the number of routes in the system
std::size_t CCSVBusSystem::RouteCount() const noexcept {
    return DImplementation->RouteCount();
}

// Returns the stop specified by the index
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept {
    return DImplementation->StopByIndex(index);
}

// Returns the stop specified by the stop id
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(TStopID id) const noexcept {
    return DImplementation->StopByID(id);
}

// Returns the route specified by the index
std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept {
    return DImplementation->RouteByIndex(index);
}

// Returns the route specified by the name
std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept {
    return DImplementation->RouteByName(name);
}
