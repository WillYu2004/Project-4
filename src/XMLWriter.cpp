#include "XMLWriter.h"
#include "XMLEntity.h"
#include "StringDataSink.h"
struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> Sink;
    std::vector<std::string> OpenElements;

    SImplementation(std::shared_ptr<CDataSink> sink) : Sink(sink) {}

    bool WriteEntity(const SXMLEntity &entity) {
        std::string entityData;
        switch (entity.DType) {
            case SXMLEntity::EType::StartElement:
                entityData = "<" + entity.DNameData;
                for (const auto &attribute : entity.DAttributes) {
                    entityData += " " + attribute.first + "=\"" + attribute.second + "\"";
                }
                entityData += ">";
                OpenElements.push_back(entity.DNameData);
                break;
            case SXMLEntity::EType::EndElement:
                if (!OpenElements.empty() && OpenElements.back() == entity.DNameData) {
                    OpenElements.pop_back();
                }
                entityData = "</" + entity.DNameData + ">";
                break;
            case SXMLEntity::EType::CharData:
                entityData = entity.DNameData; // Assuming DNameData holds the character data
                break;
            case SXMLEntity::EType::CompleteElement:
                break;
        }
        return Sink->Write(std::vector<char>(entityData.begin(), entityData.end()));
    }

    bool Flush() {
        while (!OpenElements.empty()) {
            std::string endTag = "</" + OpenElements.back() + ">";
            Sink->Write(std::vector<char>(endTag.begin(), endTag.end()));
            OpenElements.pop_back();
        }
        return true;
    }
};

CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink) : DImplementation(std::make_unique<SImplementation>(sink)) {}

CXMLWriter::~CXMLWriter() {}

bool CXMLWriter::Flush() {
    return DImplementation->Flush();
}

bool CXMLWriter::WriteEntity(const SXMLEntity &entity) {
    return DImplementation->WriteEntity(entity);
}