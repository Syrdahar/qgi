#ifndef QGI_H
#define QGI_H

#include <string>

namespace QGI {

    struct Header
    {
        unsigned width,height,length; // length = width*height*4 for rgba images
    };

    void Write(std::string path, Header& header, char*& bytes);
    void Read (std::string path, Header& header, char*& bytes);

    const unsigned long long QGI_EOF = 0x0000000000000001;
    const unsigned QGI_SIGNATURE = 0x71676966;

    const unsigned char QGI_OP_RGB = 0xc0;
    const unsigned char QGI_OP_RGBA = 0xc1;
    const unsigned char QGI_OP_LONGRUN = 0xff;

    const unsigned char QGI_OP_INDEX = 0x00;
    const unsigned char QGI_OP_DIFF = 0x40;
    const unsigned char QGI_OP_LUMA = 0x80;
    const unsigned char QGI_OP_RUN = 0xc0;

}

#endif
