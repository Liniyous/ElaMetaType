#include "ElaMetaBuffer.h"

#include "ElaMetaEndian.h"
#include <QElapsedTimer>
ElaMetaBuffer::ElaMetaBuffer()
{
    _isSwapBytes = ElaMetaEndian::getIsLittleEndian();
    static constexpr int defaultSize = 1024 * 1024 * 10;
    _buffer = new char[defaultSize];
    _bufferSize = defaultSize;
}

ElaMetaBuffer::ElaMetaBuffer(int dataSize)
{
    _isSwapBytes = ElaMetaEndian::getIsLittleEndian();
    _buffer = new char[dataSize];
    _bufferSize = dataSize;
}

ElaMetaBuffer::ElaMetaBuffer(char* data, int dataSize, bool isAllowGrow)
{
    _isSwapBytes = ElaMetaEndian::getIsLittleEndian();
    _buffer = data;
    _bufferSize = dataSize;
    _isAllowGrow = isAllowGrow;
}

ElaMetaBuffer::~ElaMetaBuffer()
{
    dispose();
}

void ElaMetaBuffer::dispose()
{
    if (_isAllowGrow)
    {
        delete[] _buffer;
    }
    _buffer = nullptr;
}

void ElaMetaBuffer::setData(char* data, int dataSize, bool isAllowGrow)
{
    dispose();
    _buffer = data;
    _bufferSize = dataSize;
    _readPos = 0;
    _writePos = dataSize;
    _isAllowGrow = isAllowGrow;
}

char* ElaMetaBuffer::getBuffer()
{
    return _buffer;
}

const char* ElaMetaBuffer::getBuffer() const
{
    return _buffer;
}

int ElaMetaBuffer::getBufferSize() const
{
    return _bufferSize;
}

void ElaMetaBuffer::setWritePos(int pos)
{
    _writePos = pos;
}

int ElaMetaBuffer::getWritePos() const
{
    return _writePos;
}

void ElaMetaBuffer::setReadPos(int pos)
{
    _readPos = pos;
}

int ElaMetaBuffer::getReadPos() const
{
    return _readPos;
}

void ElaMetaBuffer::writeRawData(const char* buffer, int dataSize)
{
    checkPutSpace(dataSize);
    memcpy(_buffer + _writePos, buffer, dataSize);
    _writePos += dataSize;
}

void ElaMetaBuffer::prependRawData(const char* buffer, int dataSize)
{
    checkPutSpace(dataSize);
    move(0, _writePos, dataSize);
    memcpy(_buffer, buffer, dataSize);
    _writePos += dataSize;
}

void ElaMetaBuffer::readRawData(char* buffer, int dataSize)
{
    memcpy(buffer, _buffer + _readPos, dataSize);
    _readPos += dataSize;
}

void ElaMetaBuffer::reset()
{
    _readPos = _writePos = 0;
}

void ElaMetaBuffer::grow(int newSize)
{
    if (newSize >= _bufferSize)
    {
        growBy(newSize - _bufferSize);
    }
}

void ElaMetaBuffer::growBy(int growSize)
{
    int newSize = growSize + _bufferSize + 1;
    if ((growSize + 1) < (_bufferSize / 2))
    {
        newSize = _bufferSize + _bufferSize / 2;
    }
    char* newData = new char[newSize];
    std::copy_n(_buffer, _bufferSize, newData);
    delete[] _buffer;
    _buffer = newData;
    _bufferSize = newSize;
}

int ElaMetaBuffer::getValidSize() const
{
    return _writePos - _readPos;
}

void ElaMetaBuffer::swapBuffer(ElaMetaBuffer& rhs)
{
    std::swap(_buffer, rhs._buffer);
    std::swap(_bufferSize, rhs._bufferSize);
    std::swap(_readPos, rhs._readPos);
    std::swap(_writePos, rhs._writePos);
    std::swap(_isAllowGrow, rhs._isAllowGrow);
}

void* ElaMetaBuffer::releaseBuffer()
{
    void* bufferPtr = _buffer;
    _buffer = nullptr;
    _bufferSize = 0;
    return bufferPtr;
}

void ElaMetaBuffer::checkPutSpace(int putSize)
{
    if (_isAllowGrow && (putSize + _writePos >= _bufferSize))
    {
        growBy(putSize);
    }
}

void ElaMetaBuffer::move(int beginOffset, int endOffset, int newBegin)
{
    if (beginOffset > newBegin)
    {
        std::copy(_buffer + beginOffset, _buffer + endOffset, _buffer + newBegin);
    }
    else if (beginOffset < newBegin)
    {
        std::copy_backward(_buffer + beginOffset, _buffer + endOffset, _buffer + newBegin + endOffset - beginOffset);
    }
}
