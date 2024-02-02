#include "qoi.h"
#include <fstream>

u8 lookupIndex(char* rgba)
{
    return ((u8)rgba[0]*3u + (u8)rgba[1]*5u + (u8)rgba[2]*7u + (u8)rgba[3]*11u)%64u;
}


void qoi::Write(Header &header, char *&bytes){
    //?
}

void qoi::Read(Header &header, char *&bytes) {
    std::ifstream file(path, std::ios::binary|std::ios::in);


    //read signature
    u32 read_signature;
    READ_BYTES(file, read_signature)
    if (read_signature != QOI_SIGNATURE){
        delete [] bytes;
        throw std::invalid_argument("wrong file format (bad SIGNATURE)");
    }

    //read header
    READ_BYTES(file, header.width)
    READ_BYTES(file, header.height)
    file.read((char*)&header.channels, 1);
    file.read((char*)&header.colorspace, 1);

    // init lookupTable
    u32 lookupTable[64] = {0u}; // 4bytes : r g b a = 32bits

    // init bytes chain (note that the chain need to be cleared before read usage)
    header.length = header.width * header.height * 4;
    bytes = new char[header.length];

    // go through the bytes chain
    u8 byte, flag, arg;
    for (char* pointer = bytes; pointer<bytes+header.length; pointer+=4) // +=4 for (r g b a)
    {
        file.read((char*)&byte,1);
        flag = 0b11000000&byte;
        arg = 0b11&byte;

        // check for 8 bits flag
        if (byte == QOI_OP_RGB)
        {
            file.read(pointer, 3); // RGB
            *(pointer+3) = pointer==bytes ? 0xff : *(pointer-1); // A

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (byte == QOI_OP_RGBA)
        {
            file.read(pointer, 4); // RGB

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }

        // check for 2 bits flag
        else if (flag == QOI_OP_INDEX)
        {
            *((u32*)pointer) = lookupTable[lookupIndex(pointer)]; // copy the value
        }
        else if (flag == QOI_OP_DIFF)
        {
            *((u32*)pointer) = *((u32*)(pointer-4)); // copy the last
            *pointer += (char)(arg>>4);//dr
            *(pointer+1) += (char)((arg>>2)&0b11);//dg
            *(pointer+2) += (char)(arg&0b11);//db
            //A stays unchanged

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (flag == QOI_OP_LUMA)
        {
            *((u32*)pointer) = *((u32*)(pointer-4)); // copy the last
            *(pointer+1) += (char)arg;//dg

            file.read((char*)&byte, 1);

            *pointer += (char)(byte>>4) + (char)arg;//dr
            *(pointer+2) += (char)(byte&0b1111) + (char)arg;//db
            //A stays unchanged

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (flag == QOI_OP_RUN)
        {
            for (u8 i = 0; i<=(u8)arg; i++) { // <= because arg is stored with a bias of -1
                *((u32*)pointer) = *((u32*)(pointer-4));
            }
        }
    }

    // read eof
    u64 read_eof;
    READ_BYTES(file, read_eof)
    if (read_eof != QOI_EOF) {
        //delete [] bytes;
        //throw std::invalid_argument("wrong file format (bad EOF)");
    }


    file.close();
}