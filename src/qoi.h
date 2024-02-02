#ifndef QOI
#define QOI

#include "format.h"



class qoi : public Format {
public:
    using Format::Format;
    void Write(Header& header, char*& bytes) override;
    void Read (Header& header, char*& bytes) override;
private:
    const u64 QOI_EOF = 0x0000000000000001;
    const u32 QOI_SIGNATURE = 0x716f6966;

    const u8 QOI_OP_RGB = 0xfe;
    const u8 QOI_OP_RGBA = 0xff;
    const u8 QOI_OP_INDEX = 0x00;
    const u8 QOI_OP_DIFF = 0x40;
    const u8 QOI_OP_LUMA = 0x80;
    const u8 QOI_OP_RUN = 0xc0;
};


#endif
