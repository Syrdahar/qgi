#include <iostream>
#include <fstream>
#include "qoi.h"

using namespace std;

Header header;
char* bytes;

char gradient[8] = {' ', '.', ',', ':', ';', 'o', '0', '@'};
char get_gradient_char(char* rgba)
{
    return gradient[((u8)rgba[0] + (u8)rgba[1] + (u8)rgba[2]) * (u8)rgba[3] * 8u / (256u*256u*3u)];
}

int main()
{
    qoi dice("../images/dice.qoi");
    qoi image("../images/test.qoi");


    dice.Read(header, bytes);

    image.Write(header, bytes);
    image.Read(header, bytes);



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