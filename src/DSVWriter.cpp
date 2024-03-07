#include "DSVWriter.h"
#include "DataSink.h"
#include "DataSource.h"
#include "StringUtils.h"

struct CDSVWriter::SImplementation{
    std::shared_ptr<CDataSink> sink;
    char delimiter;
    bool quoteall;
    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
        :sink(std::move(sink)),delimiter(delimiter),quoteall(quoteall){}
    bool WriteRow(const std::vector<std::string> &row) {
    std::string line;
    for (size_t i = 0; i < row.size(); ++i) {
        std::string element = row[i];
        bool needsQuoting = quoteall || element.find(delimiter) != std::string::npos || 
                            element.find('\"') != std::string::npos || 
                            element.find('\n') != std::string::npos;

        if (needsQuoting) {
            line += '\"' + StringUtils::Replace(element, "\"", "\"\"") + '\"'; // Escape quotes and wrap in quotes
        } else if (!quoteall) {
            line += element;
        }

        if (i < row.size() - 1) {
            line += delimiter; // Add delimiter between elements
        }
    }

    // Write the constructed line to the sink
    for (char c : line) {
        sink->Put(c);
    }

    return true; // Indicate successful writing
}
};
CDSVWriter::CDSVWriter(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall){
    DImplementation=std::make_unique<SImplementation>(sink,delimiter,quoteall);
}
CDSVWriter::~CDSVWriter(){}

bool CDSVWriter::WriteRow(const std::vector<std::string> &row){
    return DImplementation->WriteRow(row);
}