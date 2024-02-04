#include <iostream>
#include "qoi.h"
#include "qgi.h"

using namespace std;

#define u8 unsigned char
#define u32 unsigned

namespace BENCH
{
    static void perf_t(string path, string format) {
        char *bytes_;
        double diff;
        time_t start_t;
        if (format == "QOI") {
            QOI::Header header;
            // read
            start_t = time(nullptr);
            for (u32 i = 0; i < 10000u; i++) {
                QOI::Read(path, header, bytes_);
            }
            diff = difftime(time(nullptr), start_t);
            delete[] bytes_; //delete after not to influence read time, small optimization
            cout << "read perf : " << diff / 10000. << "s\n";

            //write
            QOI::Read(path, header, bytes_);
            start_t = time(nullptr);
            for (u32 i = 0; i < 10000u; i++) {
                QOI::Write(path, header, bytes_);
            }
            diff = difftime(time(nullptr), start_t);
            cout << "write perf : " << diff / 10000. << "s\n";
        } else if (format == "QGI") {
            QGI::Header header;
            // read
            start_t = time(nullptr);
            for (u32 i = 0; i < 10000u; i++) {
                QGI::Read(path, header, bytes_);
            }
            diff = difftime(time(nullptr), start_t);
            delete[] bytes_;
            cout << "read perf : " << diff / 10000. << "s\n";

            //write
            QGI::Read(path, header, bytes_);
            start_t = time(nullptr);
            for (u32 i = 0; i < 10000u; i++) {
                QGI::Write(path, header, bytes_);
            }
            diff = difftime(time(nullptr), start_t);
            cout << "write perf : " << diff / 10000. << "s\n";
        }


        delete[] bytes_;
    }
}
