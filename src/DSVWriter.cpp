#include "DSVWriter.h"
#include "DataSink.h"
#include "DataSource.h"

struct CDSVWriter::SImplementation{
    std::shared_ptr<CDataSink> sink;
    char delimiter;
    bool quoteall;
    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
        :sink(std::move(sink)),delimiter(delimiter),quoteall(quoteall){}
    bool WriteRow(const std::vector<std::string> &row) {
    if (row.empty()) {
        // Handle the case where the input vector is empty
        return false;
    }
    std::string line;
    for (size_t i = 0; i < row.size(); ++i) {
        line += row[i];
        if (i < row.size() - 1) {
            // Add the delimiter after each element except the last one
            line += delimiter;
        }
    }
    for(size_t i=0; i<line.size(); i++){
        sink->Put(line[i]);
    }

    return true;
}
};
CDSVWriter::CDSVWriter(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall){
    DImplementation=std::make_unique<SImplementation>(sink,delimiter,quoteall);
}
CDSVWriter::~CDSVWriter(){}

bool CDSVWriter::WriteRow(const std::vector<std::string> &row){
    return DImplementation->WriteRow(row);
}