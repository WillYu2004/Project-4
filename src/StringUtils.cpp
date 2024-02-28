#include "StringUtils.h"
#include <sstream>
#include <algorithm>
namespace StringUtils{

std::string Slice(const std::string &str, ssize_t start, ssize_t end) noexcept{
    ssize_t len=str.size();
    if(start<0) {
        start+=len;
        if(start<0) {
            start=0;
        }
    }
    if(end<=0) {
        end+=len;
    }
    if(start>len) {
        start=len;
    }
    if(end>len) {
        end=len;
    }
    ssize_t newLen=end-start;
    if(newLen<=0 || start>=len) {
        return "";
    }
    return str.substr(start, end-start);
}

std::string Capitalize(const std::string &str) noexcept{
if(str.empty()){
        return "";
    }
    std::string result=str;
    result[0]=std::toupper(result[0]);
    std::transform(result.begin()+1, result.end(), result.begin()+1,
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}


std::string Upper(const std::string &str) noexcept {
    std::string result=str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string Lower(const std::string &str) noexcept {
    std::string result=str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string LStrip(const std::string &str) noexcept {
    size_t start=str.find_first_not_of(" \t\n\r\f\v");
    if(start==std::string::npos) {
        return "";
    }
    return str.substr(start);
}

std::string RStrip(const std::string &str) noexcept {
    size_t end=str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(0, end+1);
}

std::string Strip(const std::string &str) noexcept {
    size_t start=str.find_first_not_of(" \t\n\r\f\v");
    if(start==std::string::npos) {
        return "";
    }
    size_t end=str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end-start+1);
}

std::string Center(const std::string &str, int width, char fill) noexcept{
     if (width <= str.size()) {
        return str;
    }
    
    int space = width - str.size();
    int padLeft = space / 2;
    int padRight = space - padLeft;

    return std::string(padLeft, fill) + str + std::string(padRight, fill);
}

std::string LJust(const std::string &str, int width, char fill) noexcept {
    if(width<=str.size()) {
        return str;
    }
    int space=width-str.size();
    return str+std::string(space, fill);
}

std::string RJust(const std::string &str, int width, char fill) noexcept {
    if(width<=str.size()) {
        return str;
    }
    int space=width-str.size();
    return std::string(space, fill)+str;
}

std::string Replace(const std::string &str, const std::string &old, const std::string &rep) noexcept{
    if(old.empty()) {
        return str;
    }

    std::string result;
    size_t start=0;
    size_t pos;
    while((pos=str.find(old, start))!=std::string::npos) {
        result+=str.substr(start, pos-start)+rep;
        start=pos+old.size();
    }
    result+=str.substr(start);
    return result;
}

std::vector<std::string> Split(const std::string &str, const std::string &splt) noexcept{
    std::vector<std::string> result;
    
    if(splt.empty()) {
        std::istringstream iss(str);
        for(std::string elem; iss >> elem;) {
            result.push_back(elem);
        }
    } else {
        size_t start=0;
        size_t end=str.find(splt);
        while(end!=std::string::npos) {
            result.push_back(str.substr(start, end-start));
            start=end+splt.length();
            end=str.find(splt, start);
        }
        result.push_back(str.substr(start, end));
    }

    return result;
}


std::string Join(const std::string &str, const std::vector< std::string > &vect) noexcept{
    std::string result;
    result+=str;
    for(const auto &elem: vect){
        result+=elem;
    }
    return result;
}

std::string ExpandTabs(const std::string &str, int tabsize) noexcept{
    std::string expanded;
    int column = 0;
    for (char ch : str) {
        if (ch == '\t') {
            int spacesNeeded = tabsize - (column % tabsize);
            expanded.append(spacesNeeded, ' ');
            column += spacesNeeded;
        } else {
            expanded.push_back(ch);
            ++column;
        }
    }
    return expanded;
}

int EditDistance(const std::string &left, const std::string &right, bool ignorecase) noexcept {
    return 0; //No idea how to do this one
}
};