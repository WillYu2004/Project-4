#include "XMLReader.h"
#include "XMLEntity.h"
#include "StringDataSource.h"
#include <expat.h>
#include <queue>
struct CXMLReader::SImplementation{
    std::shared_ptr<CDataSource> DDataSource;
    XML_Parser DXMLParser;
    std::queue<SXMLEntity>DEntityQueue;
    void StartELementHandler(const std::string &name, const std::vector<std::string> &attrs){
        SXMLEntity TempEntity;
        TempEntity.DNameData=name;
        TempEntity.DType=SXMLEntity::EType::StartElement;
        for(size_t Index=0; Index <attrs.size(); Index+=2){
            TempEntity.SetAttribute(attrs[Index], attrs[Index+1]);
        }
        DEntityQueue.push(TempEntity);
    };
    void EndElementHandler(const std::string &name){
    SXMLEntity TempEntity;
    TempEntity.DNameData = name;
    TempEntity.DType = SXMLEntity::EType::EndElement;
    DEntityQueue.push(TempEntity);
    };
    void CharacterDataHandler(const std::string &cdata){
   if (!cdata.empty()) {
        SXMLEntity TempEntity;
        TempEntity.DType = SXMLEntity::EType::CharData;
        TempEntity.DNameData = cdata;
        DEntityQueue.push(TempEntity);
    }
    };

    static void StartElementHandlerCallback(void *context, const XML_Char *name, const XML_Char **atts){
        SImplementation *ReaderObject=static_cast<SImplementation*>(context);
        std::vector<std::string> Attributes;
        auto Attrptr=atts;
        while(*Attrptr){
            Attributes.push_back(*Attrptr);
            Attrptr++;
        }
        ReaderObject->StartELementHandler(name,Attributes);
    };
    static void EndElementHandlerCallback(void *context, const XML_Char *name){
        SImplementation *ReaderObject=static_cast<SImplementation*>(context);
        ReaderObject->EndElementHandler(name);
    };
    static void CharacterDataHandlerCallback(void *context, const XML_Char *s, int len){
        SImplementation *ReaderObject=static_cast<SImplementation*>(context);
        ReaderObject->CharacterDataHandler(std::string(s,len));
    };

    SImplementation(std::shared_ptr<CDataSource> src){
        DDataSource=src;
        DXMLParser=XML_ParserCreate(NULL);
        XML_SetStartElementHandler(DXMLParser,  StartElementHandlerCallback);
        XML_SetEndElementHandler(DXMLParser, EndElementHandlerCallback);
        XML_SetCharacterDataHandler(DXMLParser, CharacterDataHandlerCallback);
        XML_SetUserData(DXMLParser,this);
    };
    bool End() const{
        return DEntityQueue.empty() && DDataSource->End();
    };
   bool ReadEntity(SXMLEntity &entity, bool skipcdata) {
    if(DEntityQueue.empty()) {
        std::vector<char> DataBuffer(1024); // Adjust buffer size as needed
        size_t ReadLength = 0;
        bool EndOfData = false;

        // Keep reading and parsing until we find an entity or run out of data
        while(DEntityQueue.empty() && !EndOfData) {
            if(DDataSource->Read(DataBuffer, DataBuffer.size())) {
                ReadLength = DataBuffer.size();
            } else {
                EndOfData = true; // No more data to read
                ReadLength = 0; // Ensure we call XML_Parse with isFinal = true
            }

            // Parse the data read from the source
            if(XML_Parse(DXMLParser, DataBuffer.data(), ReadLength, EndOfData) == XML_STATUS_ERROR) {
                // Handle parsing error (not shown here)
                return false;
            }
        }
    }

    // Now check if we have an entity in the queue
    while(!DEntityQueue.empty()) {
        const SXMLEntity& frontEntity = DEntityQueue.front();
        if(skipcdata && frontEntity.DType == SXMLEntity::EType::CharData) {
            DEntityQueue.pop(); // Skip this entity and check the next one
            continue;
        }
        // Found a suitable entity to return
        entity = frontEntity;
        DEntityQueue.pop();
        return true;
    }

    // If we get here, it means there were no suitable entities to return
    return false;
}

};
CXMLReader::CXMLReader(std::shared_ptr< CDataSource > src){
    DImplementation = std::make_unique<SImplementation>(src);
}

CXMLReader::~CXMLReader()=default;

bool CXMLReader::End() const{
    return DImplementation->End();
 }
bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata) {
    return DImplementation->ReadEntity(entity, skipcdata);
}
