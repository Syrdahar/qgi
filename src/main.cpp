#include <iostream>
#include <fstream>
#include "qoi.h"
#include "qgi.h"

using namespace std;

#define u8 unsigned char
#define u32 unsigned

QOI::Header qoiHeader;
QGI::Header qgiHeader;
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
        QOI::Header header;
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

    QOI::Read("../images/dice.qoi",qoiHeader, bytes);

    qgiHeader.width = qoiHeader.width;
    qgiHeader.height = qoiHeader.height;
    qgiHeader.length = qgiHeader.width*qgiHeader.height*4;
    QGI::Write("../images/test.qgi", qgiHeader, bytes);
    delete[] bytes;
    QGI::Read("../images/test.qgi", qgiHeader, bytes);



    //perf_t("QOI");


    std::ofstream file("../images/test.txt");
    for (u32 y = 0u; y<qgiHeader.height; y+=3)
    {
        for (u32 x = 0u; x<qgiHeader.width; x++)
        {
            file << get_gradient_char(bytes+(x+qgiHeader.width*y)*4);
        }
        file << endl;
    }
    file.close();

    delete[] bytes;

    return 0;
}