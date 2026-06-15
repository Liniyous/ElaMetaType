#ifndef ELAFRAMEWORK_ELAMETAENDIAN_H
#define ELAFRAMEWORK_ELAMETAENDIAN_H

#include "ElaSingleton.h"
class ElaMetaEndian
{
public:
    static bool getIsLittleEndian();
    static bool getIsBigEndian();

private:
    explicit ElaMetaEndian();
    ~ElaMetaEndian();
    static bool _isLittleEndian;
    static bool _isBigEndian;
    static constexpr uint16_t _checkNum = 0x1234;
};

#endif //ELAFRAMEWORK_ELAMETAENDIAN_H
