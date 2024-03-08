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
    size_t start = 0; // Start index of a new field
    bool inQuotes = false;
    for (size_t i = 0; i < line.length(); ++i) {
        if (line[i] == '\"') {
            // Toggle the inQuotes flag on encountering a quote
            inQuotes = !inQuotes;
            continue; // Skip adding or removing quotes directly to/from the field value
        }
        if (!inQuotes && line[i] == delimiter) {
            // If not in quotes and we hit a delimiter, end of a field
            row.push_back(line.substr(start, i - start)); // Add the field
            start = i + 1; // Update start to the character after the delimiter
        } else if (inQuotes && i > 0 && line[i] == '\"' && line[i-1] == '\"') {
            // Handle escaped quotes within quoted fields by removing one of the quotes
            line.erase(i, 1); // Remove the quote
            i--; // Adjust loop index since we modified the string
        }
    }
    // Add the last field if it wasn't followed by a delimiter
    if (start < line.length()) {
        row.push_back(line.substr(start));
    }

    // Post-processing to handle quoted fields correctly
    for (size_t i = 0; i < row.size(); ++i) {
        if (!row[i].empty() && row[i].front() == '\"' && row[i].back() == '\"') {
            // Remove the surrounding quotes
            row[i] = row[i].substr(1, row[i].length() - 2);

            // Replace escaped quotes ("" -> ") within the field
            size_t pos = 0; // Start of the search
            while ((pos = row[i].find("\"\"", pos)) != std::string::npos) {
                row[i].replace(pos, 2, "\"");
                pos++; // Continue searching from the next position
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