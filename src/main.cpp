#include <iostream>
#include "qoi.h"

Header header;
char* bytes;

int main()
{
    qoi file("../images/dice.qoi");


    file.Read(header, bytes);

    delete[] bytes;

    return 0;
}