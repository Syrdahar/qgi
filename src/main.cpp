#include <iostream>
#include <fstream>
#include "qoi.h"

using namespace std;

#define u8 unsigned char
#define u32 unsigned

QOI::Header header;
char* bytes;

char gradient[8] = {' ', '.', ',', ':', ';', 'o', '0', '@'};
char get_gradient_char(char* rgba)
{
    return gradient[((u8)rgba[0] + (u8)rgba[1] + (u8)rgba[2]) * (u8)rgba[3] * 8u / (256u*256u*3u)];
}

void perf_t(string format)
{
    char* bytes_;
    double diff;
    time_t start_t;
    if (format == "QOI")
    {
        // read
        start_t = time(nullptr);
        for (u32 i = 0; i< 10000u; i++)
        {
            QOI::Read("../images/test.qoi", header, bytes_);
            delete[] bytes_;
        }
        diff = difftime(time(nullptr), start_t);
        cout << "read perf : " << diff/10000. << "s\n";

        //write
        QOI::Read("../images/test.qoi", header, bytes_);
        start_t = time(nullptr);
        for (u32 i = 0; i< 10000u; i++)
        {
            QOI::Write("../images/test.qoi", header, bytes_);
        }
        diff = difftime(time(nullptr), start_t);
        cout << "write perf : " << diff/10000. << "s\n";
    }


    delete[] bytes_;
}

int main()
{

    QOI::Read("../images/dice.qoi",header, bytes);
    QOI::Write("../images/test.qoi", header, bytes);


    perf_t("QOI");


    std::ofstream file("../images/test.txt");
    for (u32 y = 0u; y<header.height; y+=3)
    {
        for (u32 x = 0u; x<header.width; x++)
        {
            file << get_gradient_char(bytes+(x+header.width*y)*4);
        }
        file << endl;
    }
    file.close();

    delete[] bytes;

    return 0;
}