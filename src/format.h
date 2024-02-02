#ifndef FORMAT
#define FORMAT

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
    virtual void Write (const char* path, Header& header, char*& bytes);
    virtual void Read (const char* path, Header& header, char*& bytes);
};


#endif
