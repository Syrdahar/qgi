#include "qgi.h"
#include <fstream>

#include <iostream>
using namespace std;
#define DISPLAY64(x) for(u8 _ = 63; _!=0xff; _--){cout << ((x&(1ull<<_))!=0);}cout << endl;

#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned
#define u64 unsigned long long

#define WRITE_BYTES(file, x) for(char* _=(char*)&x+sizeof(x)-1;_>=(char*)&x;_--){file.write(_,1);}
#define READ_BYTES(file, x) for(char* _=(char*)&x+sizeof(x)-1;_>=(char*)&x;_--){file.read(_,1);}

#define lookupIndex(rgba) (((u8)rgba[0]*3u + (u8)rgba[1]*5u + (u8)rgba[2]*7u + (u8)rgba[3]*11u)%64u)


void QGI::Write(std::string path, Header &header, char *&bytes) {
    std::ofstream file(path, std::ios::binary|std::ios::out);

    //write signature
    WRITE_BYTES(file, QGI_SIGNATURE);

    // write header
    WRITE_BYTES(file, header.width)
    WRITE_BYTES(file, header.height)

    // init lookupTable
    u32 lookupTable[64] = {0u}; // 4bytes : r g b a = 32bits

    // write the firs byte (different because there is no previous byte)
    if (*((u32*)bytes) == 0u) // color = 0 0 0 0 --> index
        file.write((char*)&QGI_OP_INDEX, 1); //index to what ever element (here 0) because they are 0 initialized
    else if (*((u8*)bytes) == 0xff) // alpha = 255 --> RGB
    {
        file.write((char*)&QGI_OP_RGB, 1);
        file.write(bytes, 3); // rgb only
    }
    else // rgba
    {
        file.write((char*)&QGI_OP_RGBA, 1);
        file.write(bytes, 4);
    }


    // go through the bytes chain
    u8 byte, arg;
    char dr,dg,db;
    for (char* pointer = bytes+4; pointer<bytes+header.length; pointer+=4) // +=4 for (r g b a)
    {
        if (*((u32*)pointer) == *((u32*)(pointer-4))) // RUN only if length >= 2 (else 1:index 0:useless)
        {
            bool longRun = false;
            u16 runLength = 0x0001;
            while (runLength!=0x00ff+0x003e && pointer+4<bytes+header.length && *((u32*)pointer) == *((u32*)(pointer+4))){
                pointer+=4;
                runLength++;
                if (runLength == 0x003f && !longRun) // max length for normal run
                {
                    runLength = 0u; // -64
                    longRun = true;
                }
            }
            if (longRun) {
                if (runLength < 0x003e) // can be written as 2 run instead of a long run
                {
                    // first run
                    byte = QGI_OP_RUN | 0x003e; // 64-1 : need to add 1 to the second run
                    file.write((char*)&byte, 1);

                    // second run
                    if (runLength == 0x0000) // can be written as an index
                    {
                        byte = QGI_OP_INDEX | lookupIndex(pointer);
                        file.write((char*)&byte, 1);
                    }
                    else // normal run
                    {
                        byte = QGI_OP_RUN | (runLength+0x0001);
                        file.write((char*)&byte, 1);
                    }
                }
                else // normal long run
                {
                    file.write((char*)&QGI_OP_LONGRUN,1);
                    byte = runLength-0x003e;
                    file.write((char*)&byte, 1);
                }
            }
            else if (runLength != 0x0001) { // normal run
                byte = QGI_OP_RUN | runLength;
                file.write((char*)&byte, 1);
            }
            else { // can be written as an index
                byte = QGI_OP_INDEX | lookupIndex(pointer); // last pixel always stored into the lookup table
                file.write((char*)&byte, 1);
            }
        }
        else if (*((u32*)pointer) == lookupTable[lookupIndex(pointer)]) // INDEX
        {
            byte = QGI_OP_INDEX | lookupIndex(pointer);
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
                byte |= QGI_OP_DIFF;

                file.write((char*)&byte,1);

                lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
            }
            else if (-8  <= dr-dg && dr-dg <= 7
                     && -32 <= dg    && dg    <= 31
                     && -8  <= db-dg && db-dg <= 7)// LUMA
            {
                byte =  QGI_OP_LUMA;
                byte |= (u8)(dg + 32);

                file.write((char*)&byte,1);

                byte =  (u8)(db - dg + 8);
                byte |= (u8)(dr - dg + 8) << 4;

                file.write((char*)&byte,1);

                lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
            }
            else // RGB
            {
                file.write((char*)&QGI_OP_RGB, 1);
                file.write(pointer, 3);

                lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
            }
        }
        else // RGBA
        {
            file.write((char*)&QGI_OP_RGBA, 1);
            file.write(pointer, 4);

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer);
        }
    }

    //write eof
    WRITE_BYTES(file, QGI_EOF);

    file.close();
}

void QGI::Read(std::string path, Header &header, char *&bytes) {
    std::ifstream file(path, std::ios::binary|std::ios::in);

    //read signature
    u32 read_signature;
    READ_BYTES(file, read_signature)
    if (read_signature != QGI_SIGNATURE){
        delete [] bytes;
        throw std::invalid_argument("READ ERROR : wrong file format (bad SIGNATURE)");
    }

    //read header
    READ_BYTES(file, header.width)
    READ_BYTES(file, header.height)

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
        if (byte == QGI_OP_RGB)
        {
            file.read(pointer, 3); // RGB
            *(pointer+3) = pointer==bytes ? (char)0xff : *(pointer-1); // A  (1 if it's the first byte else take the alpha of the previous byte)

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (byte == QGI_OP_RGBA)
        {
            file.read(pointer, 4); // RGBA

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (byte == QGI_OP_LONGRUN)
        {
            file.read((char*)&byte, 1); // read 1 more byte for the runLength

            u16 runLength = byte + 0x003f + 0x003e;
            u16 i = 0x0000;
            while (++i<runLength) {
                *((u32*)pointer) = *((u32*)(pointer-4));
                pointer+=4;
            }
            *((u32*)pointer) = *((u32*)(pointer-4));
        }

        // check for 2 bits flag
        else if (flag == QGI_OP_INDEX)
        {
            *((u32*)pointer) = lookupTable[(u8)arg]; // copy the value
        }
        else if (flag == QGI_OP_DIFF)
        {
            *((u32*)pointer) = *((u32*)(pointer-4)); // copy the last
            *pointer += (char)(arg>>4) - 2;//dr
            *(pointer+1) += (char)((arg>>2)&0b11) - 2;//dg
            *(pointer+2) += (char)(arg&0b11) - 2;//db
            //A stays unchanged

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (flag == QGI_OP_LUMA)
        {
            *((u32*)pointer) = *((u32*)(pointer-4)); // copy the last
            *(pointer+1) += (char)arg - 32;//dg

            file.read((char*)&byte, 1);

            *pointer += (char)(byte>>4) - 8 + (char)arg - 32;//dr
            *(pointer+2) += (char)(byte&0b1111) - 8 + (char)arg - 32;//db
            //A stays unchanged

            lookupTable[lookupIndex(pointer)] = *((u32*)pointer); // update lookupTable
        }
        else if (flag == QGI_OP_RUN)
        {
            u8 i = 0u;
            while (++i<arg) {
                *((u32*)pointer) = *((u32*)(pointer-4));
                pointer+=4;
            }
            *((u32*)pointer) = *((u32*)(pointer-4));
        }
    }

    // read eof
    u64 read_eof;
    READ_BYTES(file, read_eof)
    if (read_eof != QGI_EOF) {
        DISPLAY64(read_eof)
        delete [] bytes;
        throw std::invalid_argument("READ ERROR : wrong file format (bad EOF)");
    }


    file.close();
}