#include "DSVReader.h"
#include "DataSource.h"
struct CDSVReader::SImplementation{
    std::shared_ptr< CDataSource > source;
    char delimiter;
    bool AtEnd;
     SImplementation(std::shared_ptr<CDataSource> src, char delimiter)
        :source(std::move(src)), delimiter(delimiter),AtEnd(false){}
    bool End() const {
        return AtEnd;
    }

    bool ReadRow(std::vector<std::string>& row) {
    char ch;
    std::string line;
    bool dataRead = false;
    row.clear(); // Clear the passed vector to hold the new row.

    // Loop to read characters until a newline is encountered or the end of the source.
    while (!source->End() && source->Get(ch)) {
        dataRead = true; // Mark that we've started reading data.

        if (ch == '\n' || ch == '\r') {
            // Handle possible Windows-style newline sequence "\r\n".
            if (ch == '\r' && source->Peek(ch) && ch == '\n') {
                source->Get(ch); // Consume the '\n' character.
            }
            break; // Exit the loop as the end of a row is reached.
        }
        else if(ch == '\"'){
            line += nullptr;
        }
        else {
            line += ch; // Append the character to the current line.
        }
    }

    // If no data was read (and possibly the end of the source was reached), return false.
    if (!dataRead) {
        AtEnd = true; // Mark the end of the data source
        return false;
    }

    // Parsing the line considering quoted fields
    size_t start = 0;
    bool inQuotes = false;
    for (size_t i = 0; i <= line.length(); ++i) {
        if (i == line.length() || (line[i] == delimiter && !inQuotes)) {
            row.push_back(line.substr(start, i - start));
            start = i + 1; // Move past the delimiter
        } else if (line[i] == '\"') {
            // Toggle the inQuotes flag if not escaped quote
            if (!(i > 0 && line[i-1] == '\"')) { // Check for escaped quote
                inQuotes = !inQuotes;
            } else if (i > 0) {
                // Remove one of the double quotes from the escaped quote pair
                line.erase(i--, 1);
            }
        }
    }

    // If the line is not empty but no row was read, add the line as a single field row
    if (line.empty() && row.empty()) {
        row.push_back(line);
    }

    if(source->End()){
        AtEnd = true;
    }
    
    return true; // Return true as a row has been successfully read.
}

};
    bool CDSVReader::End() const {
    return DImplementation->End();
}
    CDSVReader::CDSVReader(std::shared_ptr< CDataSource > src, char delimiter)
    : DImplementation(std::make_unique<SImplementation>(src, delimiter)) {
    // Code to initialize the CDSVWriter
}
    CDSVReader::~CDSVReader()=default;

    bool CDSVReader::ReadRow(std::vector<std::string> &row) {
    return DImplementation->ReadRow(row);
}