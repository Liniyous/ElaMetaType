#ifndef ELAMETATYPE_ELAMETATYPE_PRIVATE_ELAMETABUFFER_H_
#define ELAMETATYPE_ELAMETATYPE_PRIVATE_ELAMETABUFFER_H_

#include "ElaMetaTypeExport.h"
#include <algorithm>

class ELA_METATYPE_EXPORT ElaMetaBuffer
{
public:
    explicit ElaMetaBuffer();
    explicit ElaMetaBuffer(int dataSize);
    ElaMetaBuffer(char* data, int dataSize, bool isAllowGrow = true);
    ~ElaMetaBuffer();

    void dispose();

    void setData(char* data, int dataSize, bool isAllowGrow = true);

    char* getBuffer();
    const char* getBuffer() const;

    int getBufferSize() const;

    void setWritePos(int pos);
    int getWritePos() const;

    void setReadPos(int pos);
    int getReadPos() const;

    void writeRawData(const char* buffer, int dataSize);
    void prependRawData(const char* buffer, int dataSize);
    void readRawData(char* buffer, int dataSize);

    void reset();
    void move(int beginOffset, int endOffset, int newBegin);
    void grow(int newSize);
    void growBy(int growSize);
    int getValidSize() const;

    void swapBuffer(ElaMetaBuffer& rhs);
    void* releaseBuffer();
    void checkPutSpace(int putSize);

    template <typename T>
    void writeValue(const T& data)
    {
        _writeValue<sizeof(T)>(reinterpret_cast<const char*>(&data));
    }

    template <typename T>
    void prependValue(const T& data)
    {
        _prependValue<sizeof(T)>(reinterpret_cast<const char*>(&data));
    }

    template <typename T>
    void readValue(T& data)
    {
        _readValue<sizeof(T)>(reinterpret_cast<char*>(&data));
    }

private:
    template <int BYTES>
    void _writeValue(const char* data)
    {
        checkPutSpace(BYTES);
        if (_isSwapBytes)
        {
            for (int i = 0; i < BYTES; ++i)
            {
                *(_buffer + _writePos + i) = data[BYTES - 1 - i];
            }
        }
        else
        {
            memcpy(_buffer + _writePos, data, BYTES);
        }
        _writePos += BYTES;
    }

    template <int BYTES>
    void _prependValue(const char* data)
    {
        checkPutSpace(BYTES);
        move(0, _writePos, BYTES);
        if (_isSwapBytes)
        {
            for (int i = 0; i < BYTES; ++i)
            {
                *(_buffer + i) = data[BYTES - 1 - i];
            }
        }
        else
        {
            memcpy(_buffer, data, BYTES);
        }
        _writePos += BYTES;
    }

    template <int BYTES>
    void _readValue(char* data)
    {
        if (_isSwapBytes)
        {
            char* bufPtr = _buffer + _readPos;
            for (int i = 0; i < BYTES; ++i)
            {
                *(data + i) = bufPtr[BYTES - 1 - i];
            }
        }
        else
        {
            memcpy(data, _buffer + _readPos, BYTES);
        }
        _readPos += BYTES;
    }

    char* _buffer{nullptr};
    int _bufferSize{0};
    int _readPos{0};
    int _writePos{0};
    bool _isAllowGrow{true};
    bool _isSwapBytes{true};
};

#endif //ELAMETATYPE_ELAMETATYPE_PRIVATE_ELAMETABUFFER_H_
