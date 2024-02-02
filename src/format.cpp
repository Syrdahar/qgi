#include "format.h"
#include <fstream>

void Format::Write(const char *path, Header &header, char *&bytes){ // default write func
    std::ofstream file(path, std::ios::binary|std::ios::out);
    file.write(bytes, header.length);
    file.close();
}

void Format::Read(const char *path, Header &header, char *&bytes){ // default write func
    std::ifstream file(path, std::ios::binary|std::ios::in);
    file.read(bytes, header.length);
    file.close();
}