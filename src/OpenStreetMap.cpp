#include "OpenStreetMap.h"
#include <unordered_map>
#include <vector>

struct COpenStreetMap::SImplementation{
    //SNode interface
    struct SNode : public CStreetMap::SNode{
        //SNode variables
        TNodeID DID;
        TLocation DLocation;
        std::unordered_map<std::string, std::string> DAttributes;
        std::vector<std::string> DAttributeKeys;

        //Constructor
        SNode(TNodeID id, TLocation location)
        : DID(id), DLocation(location) {
        }
        //Destructor
        ~SNode(){
        
        }

        TNodeID ID() const noexcept override{
            return DID; //Returns the id of the SNode
        }

        TLocation Location() const noexcept override{
            return DLocation; //Returns the lat/lon location of the SNode
        }

        std::size_t AttributeCount() const noexcept override{
            return DAttributeKeys.size(); //Returns the number of attributes attached to the SNode
        }

        std::string GetAttributeKey(std::size_t index) const noexcept override{
            if(index <DAttributeKeys.size()){
                return DAttributeKeys[index]; //Returns the key of the attribute at index
            }
            return std::string(); //Returns empty string if index is greater than or equal to AttributeCount()
        }

        bool HasAttribute(const std::string &key) const noexcept override{
            auto Search = DAttributes.find(key);
            return DAttributes.end() != Search; //Returns if the attribute is attached to the SNode
        }

        std::string GetAttribute(const std::string &key) const noexcept override{
            auto Search = DAttributes.find(key);
            if(DAttributes.end() != Search){
                return Search->second; //Returns the value of the attribute specified by key
            }
            return std::string(); //Returns empty string if key hasn't been attached to SNode
        }

        void SetAttribute(const std::string &key, const std::string &value){
            DAttributeKeys.push_back(key);
            DAttributes[key] = value; //Returns if the attribute is attached to the SNode
        }
    };

    //SWay Interface
    struct SWay : public CStreetMap::SWay{
        //SWay Variables
        TWayID WID;
        std::vector<TNodeID> NodeIDs;
        std::unordered_map<std::string, std::string> Attributes;
        std::vector<std::string> AttributeKeys;

        //Constructor
        SWay(){
        }

        //Destructor
        ~SWay(){

        }

        TWayID ID() const noexcept override{
            return WID; // Returns the id of the SWay
        }

        std::size_t NodeCount() const noexcept override{
            return NodeIDs.size(); // Returns the number of nodes in the way
        }

        TNodeID GetNodeID(std::size_t index) const noexcept override{
            if(index < NodeIDs.size()){
                return NodeIDs[index]; //Returns the node id of the node at index
            }
            return InvalidNodeID; //Returns InvalidNodeID if index is greater than or equal to NodeCount()
        }

        std::size_t AttributeCount() const noexcept override{
            return Attributes.size(); //Returns the number of attributes attached to the SWay
        }

        std::string GetAttributeKey(std::size_t index) const noexcept override{
            if(index < AttributeKeys.size()){
                return AttributeKeys[index]; //Returns the key of the attribute at index
            }
            return std::string(); //Returns empty string if index is greater than or equal to AttributeCount()
        }

        bool HasAttribute(const std::string &key) const noexcept override{
            return Attributes.find(key) != Attributes.end(); //Returns if the attribute is attached to the SWay
        }

        std::string GetAttribute(const std::string &key) const noexcept override{
            auto it = Attributes.find(key);
            if (it != Attributes.end()) {
                return it->second; //Returns the value of the attribute specified by key
            }
            return std::string(); //Returns empty string if key hasn't been attached to SWay
        }
    };

    std::unordered_map<TNodeID, std::shared_ptr<CStreetMap::SNode> > DNodeIDToNode;
    std::vector< std::shared_ptr<CStreetMap::SNode> > DNodesByIndex;
    std::unordered_map<TWayID, std::shared_ptr<CStreetMap::SWay> > DWayIDToWay;
    std::vector< std::shared_ptr<CStreetMap::SWay> > DWaysByIndex;

    SImplementation(std::shared_ptr<CXMLReader> src){
        SXMLEntity TempEntity;

        while(src->ReadEntity(TempEntity,true)){
            if((TempEntity.DNameData == "osm")&&(SXMLEntity::EType::EndElement == TempEntity.DType)){
                //reached end
                break;
            }
            else if((TempEntity.DNameData == "node")&&(SXMLEntity::EType::StartElement == TempEntity.DType)){
                //parse node
                TNodeID NewNodeID = std::stoull(TempEntity.AttributeValue("id"));
                double Lat = std::stod(TempEntity.AttributeValue("lat"));
                double Lon = std::stod(TempEntity.AttributeValue("lon"));
                TLocation NewNodeLocation = std::make_pair(Lat,Lon);
                auto NewNode = std::make_shared<SNode>(NewNodeID,NewNodeLocation);
                DNodesByIndex.push_back(NewNode);
                DNodeIDToNode[NewNodeID] = NewNode;
                while(src->ReadEntity(TempEntity,true)){
                    if((TempEntity.DNameData == "node")&&(SXMLEntity::EType::EndElement == TempEntity.DType)){
                        break;
                    }
                    else if((TempEntity.DNameData == "tag")&&(SXMLEntity::EType::StartElement == TempEntity.DType)){
                        NewNode->SetAttribute(TempEntity.AttributeValue("k"),TempEntity.AttributeValue("v"));
                    }
                }
            }

            else if((TempEntity.DNameData == "way")&&(SXMLEntity::EType::StartElement == TempEntity.DType)){
                //parse way
                TWayID NewWayID = std::stoull(TempEntity.AttributeValue("id"));
                auto NewWay = std::make_shared<SWay>();
                NewWay->WID = NewWayID;

                // Read nested entities within the way element
                while(src->ReadEntity(TempEntity, true)){
                    // If it's the end of the way element, break the loop
                    if((TempEntity.DNameData == "way") && (SXMLEntity::EType::EndElement == TempEntity.DType)){
                        break;
                    }
                    // If it's a node reference (nd element), add the node ID to the way's node list
                    else if((TempEntity.DNameData == "nd") && (SXMLEntity::EType::StartElement == TempEntity.DType)){
                        TNodeID NodeID = std::stoull(TempEntity.AttributeValue("ref"));
                        NewWay->NodeIDs.push_back(NodeID);
                    }
                    // If it's a tag element, add the attribute to the way's attribute map
                    else if((TempEntity.DNameData == "tag") && (SXMLEntity::EType::StartElement == TempEntity.DType)){
                        std::string Key = TempEntity.AttributeValue("k");
                        std::string Value = TempEntity.AttributeValue("v");
                        NewWay->Attributes[Key] = Value;
                    }
                }
                // Once all nd and tag elements have been processed, add the way to the data structures
                DWayIDToWay[NewWayID] = NewWay;
                DWaysByIndex.push_back(NewWay);
            }
        }
    }

    std::size_t NodeCount() const noexcept{
        return DNodesByIndex.size();
    }
    
    std::size_t WayCount() const noexcept {
        return DWaysByIndex.size();
    }
    
    std::shared_ptr<CStreetMap::SNode> NodeByIndex(std::size_t index) const noexcept {
        if(index < DNodesByIndex.size()){
            return DNodesByIndex[index];
        }
        return nullptr;
    }
    
    std::shared_ptr<CStreetMap::SNode> NodeByID(TNodeID id) const noexcept {
        auto Search = DNodeIDToNode.find(id);
        if(DNodeIDToNode.end() != Search){
            return Search->second;
        }
        return nullptr;
    }
    
    std::shared_ptr<CStreetMap::SWay> WayByIndex(std::size_t index) const noexcept {
        if(index < DWaysByIndex.size()){
            return DWaysByIndex[index];
        }
        return nullptr;
    }
    
    std::shared_ptr<CStreetMap::SWay> WayByID(TWayID id) const noexcept {
        auto Search = DWayIDToWay.find(id);
        if(DWayIDToWay.end() != Search){
            return Search->second;
        }
        return nullptr;
    }
};

COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src){
    DImplementation=std::make_unique<SImplementation>(src);
}
COpenStreetMap::~COpenStreetMap()=default;

std::size_t COpenStreetMap::NodeCount() const noexcept{
    return DImplementation->NodeCount();
}
std::size_t COpenStreetMap::WayCount() const noexcept{
     return DImplementation->WayCount();
}
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept{
     return DImplementation->NodeByIndex(index);
}
 std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept {
    return DImplementation->NodeByID(id);
 }
 std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept {
    return DImplementation->WayByIndex(index);
 }
 std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept {
    return DImplementation->WayByID(id);
 }