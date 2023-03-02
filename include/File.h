#ifndef __FILE_H__
#define __FILE_H__

#include <fstream>

namespace zx
{

class File : public std::fstream
{
public:
    size_t read(uint8_t* buf, size_t size);
    bool seek(uint32_t off, ios_base::seekdir way);
    size_t write(const uint8_t *buf, size_t size);
};

}

#endif /* __FILE_H__ */
