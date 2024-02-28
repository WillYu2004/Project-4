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
        } else {
            line += ch; // Append the character to the current line.
        }
    }

    // If no data was read (and possibly the end of the source was reached), return false.
    if (!dataRead) {
        return false;
    }

    // Split the line into columns based on the delimiter and populate the row vector.
    size_t start = 0;
    size_t end = line.find(delimiter);
    while (end != std::string::npos) {
        row.push_back(line.substr(start, end - start));
        start = end + 1;
        end = line.find(delimiter, start);
    }
    row.push_back(line.substr(start)); // Add the last column if there's no delimiter at the end of the line.

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