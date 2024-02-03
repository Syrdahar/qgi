#ifndef QOI_H
#define QOI_H

#include <string>

namespace QOI {

    struct Header
    {
        unsigned width,height,length; // length = width*height*4 for images
        unsigned char channels, colorspace;
    };

    void Write(std::string path, Header& header, char*& bytes);
    void Read (std::string path, Header& header, char*& bytes);

    const unsigned long long QOI_EOF = 0x0000000000000001;
    const unsigned QOI_SIGNATURE = 0x716f6966;

    const unsigned char QOI_OP_RGB = 0xfe;
    const unsigned char QOI_OP_RGBA = 0xff;
    const unsigned char QOI_OP_INDEX = 0x00;
    const unsigned char QOI_OP_DIFF = 0x40;
    const unsigned char QOI_OP_LUMA = 0x80;
    const unsigned char QOI_OP_RUN = 0xc0;
};


#endif
