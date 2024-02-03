#include "qoi.h"
#include <fstream>

#define u8 unsigned char
#define u32 unsigned
#define u64 unsigned long long

#define WRITE_BYTES(file, x) for(char* _=(char*)&x+sizeof(x)-1;_>=(char*)&x;_--){file.write(_,1);}
#define READ_BYTES(file, x) for(char* _=(char*)&x+sizeof(x)-1;_>=(char*)&x;_--){file.read(_,1);}

u8 lookupIndex(char* rgba)
{
    return ((u8)rgba[0]*3u + (u8)rgba[1]*5u + (u8)rgba[2]*7u + (u8)rgba[3]*11u)%64u;
}


void QOI::Write(std::string path, Header &header, char *&bytes) {
    std::ofstream file(path, std::ios::binary|std::ios::out);

    //write signature
    WRITE_BYTES(file, QOI_SIGNATURE);

    // write header
    WRITE_BYTES(file, header.width)
    WRITE_BYTES(file, header.height)
    file.write((char*)&header.channels, 1);
    file.write((char*)&header.colorspace, 1);

    // init lookupTable
    u32 lookupTable[64] = {0u}; // 4bytes : r g b a = 32bits

    // write the firs byte (different because there is no previous byte)
    if (*((u32*)bytes) == 0u) // color = 0 0 0 0 --> index
        file.write((char*)&QOI_OP_INDEX, 1); //index to what ever element (here 0) because they are 0 initialized
    else if (*((u8*)bytes) == 0xff) // alpha = 255 --> RGB
    {
        file.write((char*)&QOI_OP_RGB, 1);
        file.write(bytes, 3); // rgb only
    }
    else // rgba
    {
        file.write((char*)&QOI_OP_RGBA, 1);
        file.write(bytes, 4);
    }


    // go through the bytes chain
    u8 byte, arg;
    char dr,dg,db;
    for (char* pointer = bytes+4; pointer<bytes+header.length; pointer+=4) // +=4 for (r g b a)
    {
        if (*((u32*)pointer) == *((u32*)(pointer-4))) // RUN
        {
            arg = 0u; // = 1 (stored with a bias of -1)
            while (*((u32*)pointer) == *((u32*)(pointer+4)) && arg!=0b111101){
                arg++;
                pointer+=4;
            }
            byte = QOI_OP_RUN | arg;
            file.write((char*)&byte, 1);
        }
        else if (*((u32*)pointer) == lookupTable[lookupIndex(pointer)]) // INDEX
        {
            byte = QOI_OP_INDEX | lookupIndex(pointer);
            file.write((char*)&byte, 1);
        }
        else if (*(pointer+3) == *(pointer-1))
        {
            dr = *pointer    -*(pointer-4);
            dg = *(pointer+1)-*(pointer-3);
            db = *(pointer+2)-*(pointer-2);

            if (-2 <= dr && dr <= 1
                && -2 <= dg && dg <= 1
                && -2 <= db && db <= 1)// DIFF
            {
                byte =  (u8)(db +2);
                byte |= (u8)(dg +2) << 2;
                byte |= (u8)(dr +2) << 4;
                byte |= QOI_OP_DIFF;

                file.write((char*)&byte,1);

                lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
            }
            else if (-8  <= dr-dg && dr-dg <= 7
                     && -32 <= dg    && dg    <= 31
                     && -8  <= db-dg && db-dg <= 7)// LUMA
            {
                byte =  QOI_OP_LUMA;
                byte |= (u8)(dg + 32);

                file.write((char*)&byte,1);

                byte =  (u8)(db - dg + 8);
                byte |= (u8)(dr - dg + 8) << 4;

                file.write((char*)&byte,1);

                lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
            }
            else // RGB
            {
                file.write((char*)&QOI_OP_RGB, 1);
                file.write(pointer, 3);

                lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
            }
        }
        else // RGBA
        {
            file.write((char*)&QOI_OP_RGBA, 1);
            file.write(pointer, 4);

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
        }
    }

    //write eof
    WRITE_BYTES(file, QOI_EOF);

    file.close();
}

void QOI::Read(std::string path, Header &header, char *&bytes) {
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
    header.length = header.width * header.height * 4u;
    bytes = new char[header.length];

    // go through the bytes chain
    u8 byte, flag, arg;
    for (char* pointer = bytes; pointer<bytes+header.length; pointer+=4) // +=4 for (r g b a)
    {
        file.read((char*)&byte,1);
        flag = 0b11000000&byte;
        arg = 0b00111111&byte;

        // check for 8 bits flag
        if (byte == QOI_OP_RGB)
        {
            file.read(pointer, 3); // RGB
            *(pointer+3) = pointer==bytes ? (char)0xff : *(pointer-1); // A  (1 if it's the first byte else take the alpha of the previous byte)

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (byte == QOI_OP_RGBA)
        {
            file.read(pointer, 4); // RGBA

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }

        // check for 2 bits flag
        else if (flag == QOI_OP_INDEX)
        {
            *((u32*)pointer) = lookupTable[(u8)arg]; // copy the value
        }
        else if (flag == QOI_OP_DIFF)
        {
            *((u32*)pointer) = *((u32*)(pointer-4)); // copy the last
            *pointer += (char)(arg>>4) - 2;//dr
            *(pointer+1) += (char)((arg>>2)&0b11) - 2;//dg
            *(pointer+2) += (char)(arg&0b11) - 2;//db
            //A stays unchanged

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (flag == QOI_OP_LUMA)
        {
            *((u32*)pointer) = *((u32*)(pointer-4)); // copy the last
            *(pointer+1) += (char)arg - 32;//dg

            file.read((char*)&byte, 1);

            *pointer += (char)(byte>>4) - 8 + (char)arg - 32;//dr
            *(pointer+2) += (char)(byte&0b1111) - 8 + (char)arg - 32;//db
            //A stays unchanged

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (flag == QOI_OP_RUN)
        {
            u8 i = 0;
            while (i++<arg) {
                *((u32*)pointer) = *((u32*)(pointer-4));
                pointer+=4;
            }
            *((u32*)pointer) = *((u32*)(pointer-4));// <= because arg is stored with a bias of -1
        }
    }

    // read eof
    u64 read_eof;
    READ_BYTES(file, read_eof)
    if (read_eof != QOI_EOF) {
        delete [] bytes;
        throw std::invalid_argument("wrong file format (bad EOF)");
    }


    file.close();
}