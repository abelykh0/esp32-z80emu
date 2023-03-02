#include "File.h"
#include "settings.h"
#include "esp_log.h"

using namespace zx;

size_t File::read(uint8_t* buf, size_t size)
{
    int pos1 = this->tellg();
    reinterpret_cast<std::istream*>(this)->read((char *)buf, size);
    int pos2 = this->tellg();
    return pos2 - pos1;
}

size_t File::write(const uint8_t *buf, size_t size)
{
    int pos1 = this->tellp();
    reinterpret_cast<std::ostream*>(this)->write((char *)buf, size);
    int pos2 = this->tellp();
    return pos2 - pos1;
}

bool File::seek(uint32_t off, ios_base::seekdir way)
{
    this->seekg(off, way);
    return !this->bad();
}
