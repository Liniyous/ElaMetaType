#include "ElaMetaEndian.h"

bool ElaMetaEndian::_isLittleEndian = *reinterpret_cast<const uint8_t*>(&_checkNum) == 0x34;
bool ElaMetaEndian::_isBigEndian = *reinterpret_cast<const uint8_t*>(&_checkNum) == 0x12;

bool ElaMetaEndian::getIsLittleEndian()
{
    return _isLittleEndian;
}

bool ElaMetaEndian::getIsBigEndian()
{
    return _isBigEndian;
}

ElaMetaEndian::ElaMetaEndian()
{
}

ElaMetaEndian::~ElaMetaEndian()
{
}