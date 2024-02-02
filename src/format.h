#ifndef FORMAT
#define FORMAT

#include <string>

#define u8 unsigned char
#define u32 unsigned
#define u64 unsigned long long


#define WRITE_BYTES(file, x) for(char* _=(char*)&x+sizeof(x)-1;_>=(char*)&x;_--){file.write(_,1);}
#define READ_BYTES(file, x) for(char* _=(char*)&x+sizeof(x)-1;_>=(char*)&x;_--){file.read(_,1);}


struct Header
{
    u32 width,height,length; // length = width*height*4 for images
    u8 channels, colorspace;
};

class Format {
public:
    explicit Format(std::string path);
    virtual void Write (Header& header, char*& bytes);
    virtual void Read (Header& header, char*& bytes);
protected:
    std::string path;
};


#endif
