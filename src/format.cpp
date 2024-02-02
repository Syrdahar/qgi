#include "format.h"
#include <fstream>
#include <string>

Format::Format(std::string path_) {
    path = path_;
}

void Format::Write(Header &header, char *&bytes){ // default write func
    std::ofstream file(path, std::ios::binary|std::ios::out);
    file.write((char*)&header.length, 4);
    file.write(bytes, header.length);
    file.close();
}

void Format::Read(Header &header, char *&bytes){ // default write func
    std::ifstream file(path, std::ios::binary|std::ios::in);
    file.read((char*)&header.length, 4);
    file.read(bytes, header.length);
    file.close();
}