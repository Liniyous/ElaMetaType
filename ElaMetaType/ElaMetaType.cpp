#include "ElaMetaType.h"

#include "ElaMetaBuffer.h"
#include "ElaMetaTypePrivate.h"
#include "lz4.h"
#include "pugixml.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <sstream>
#include <utility>
Q_SINGLETON_CREATE_CPP(ElaMetaType)

ElaMetaType::ElaMetaType(QObject* parent)
    : QObject(parent), d_ptr(new ElaMetaTypePrivate())
{
    Q_D(ElaMetaType);
    d->q_ptr = this;
}

ElaMetaType::~ElaMetaType()
{
}

QString ElaMetaType::serialize(const ElaVariant& variant)
{
    Q_D(ElaMetaType);
    if (!variant.isValid())
    {
        qDebug() << "ElaMetaType: Serialize Variant Is Invalid!";
        return {};
    }
    pugi::xml_document doc;
    pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    doc.append_child(pugi::node_comment).set_value("This XML File Build From ElaMetaType Copyright (c) 2025 Liniyous");
    auto rootNode = doc.append_child("ElaMetaType_Root");
    rootNode.append_attribute("Version").set_value("1.0.0");

    const auto& metaTypeInfo = d->_getMetaTypeInfo(variant.getMetaHash());
    if (!metaTypeInfo._isValid)
    {
        qDebug() << "ElaMetaType: Serialize Variant Is Not Registered!";
        return {};
    }
    auto metaNode = rootNode.append_child(metaTypeInfo._metaTypeName.toUtf8().data());
    d->_serializeMetaType(metaTypeInfo, variant, metaNode);

    std::stringstream docStringStream;
    doc.save(docStringStream);
    return QString::fromUtf8(docStringStream.str().c_str());
}

ElaVariant ElaMetaType::deserialize(const QString& serialData)
{
    Q_D(ElaMetaType);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(serialData.toUtf8());
    if (!result)
    {
        qDebug() << "ElaMetaType: SerialData Parse Failed!";
        return {};
    }
    pugi::xml_node rootNode = doc.child("ElaMetaType_Root");
    if (!rootNode)
    {
        qDebug() << "ElaMetaType: ElaMetaType_Root Not Found!";
        return {};
    }
    if (QString::fromUtf8(rootNode.attribute("Version").as_string()) != d->_version)
    {
        qDebug() << "ElaMetaType: SerialData Version Check Failed!";
        return {};
    }
    auto firstNode = rootNode.first_child();
    return d->_deserializeMetaType(firstNode);
}

QByteArray ElaMetaType::serializeToByte(const ElaVariant& variant, bool isCompress)
{
    Q_D(ElaMetaType);
    if (!variant.isValid())
    {
        qDebug() << "ElaMetaType: SerializeToByte Variant Is Invalid!";
        return {};
    }
    const auto& metaTypeInfo = d->_getMetaTypeInfo(variant.getMetaHash());
    thread_local ElaMetaBuffer metaBuffer;
    metaBuffer.reset();
    d->_serializeMetaType(metaTypeInfo, variant, metaBuffer);

    if (isCompress)
    {
        int metaBufferSize = metaBuffer.getValidSize();
        int metaBufferMaxSize = LZ4_compressBound(metaBufferSize);
        QByteArray metaByteArray(metaBufferMaxSize, Qt::Uninitialized);
        int actualSize = LZ4_compress_default(metaBuffer.getBuffer(), metaByteArray.data(), metaBufferSize, metaBufferMaxSize);
        metaByteArray.resize(actualSize);
        metaBuffer.reset();
        metaBuffer.writeValue(true);
        metaBuffer.writeValue(metaBufferSize);
        metaBuffer.writeRawData(metaByteArray.constData(), actualSize);
    }
    else
    {
        metaBuffer.prependValue(false);
    }
    return {metaBuffer.getBuffer(), metaBuffer.getValidSize()};
}

ElaVariant ElaMetaType::deserializeFromByte(QByteArray& serialData)
{
    Q_D(ElaMetaType);
    if (serialData.isEmpty())
    {
        qDebug() << "ElaMetaType: DeserializeFromByte Parse Failed!";
        return {};
    }
    thread_local ElaMetaBuffer metaBuffer;
    metaBuffer.setData(serialData.data(), serialData.size(), false);
    bool isCompress = false;
    metaBuffer.readValue(isCompress);
    if (isCompress)
    {
        int metaBufferSize = 0;
        metaBuffer.readValue(metaBufferSize);
        // 不能使用临时变量 ElaMetaBuffer直接使用原始指针 会直接被编译器优化释放内存
        thread_local QByteArray metaByteArray;
        metaByteArray = QByteArray(metaBufferSize, Qt::Uninitialized);
        int actualSize = LZ4_decompress_safe(metaBuffer.getBuffer() + metaBuffer.getReadPos(), metaByteArray.data(), metaBuffer.getValidSize(), metaBufferSize);
        metaByteArray.resize(actualSize);
        metaBuffer.setData(metaByteArray.data(), metaByteArray.size(), false);
    }
    return d->_deserializeMetaType(metaBuffer);
}

void ElaMetaType::saveToFile(const QString& fileName, const QString& serialData)
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
    auto utf8Data = serialData.toUtf8();
    saveFile.resize(utf8Data.size());
    if (uchar* fileMemory = saveFile.map(0, utf8Data.size()))
    {
        memcpy(fileMemory, utf8Data.constData(), utf8Data.size());
        saveFile.unmap(fileMemory);
    }
    else
    {
        qWarning() << "ElaMetaType: saveToFile Map Failed!";
    }
    saveFile.flush();
    saveFile.close();
}

void ElaMetaType::saveToFile(const QString& fileName, const QByteArray& serialData)
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
    saveFile.resize(serialData.size());
    if (uchar* fileMemory = saveFile.map(0, serialData.size()))
    {
        memcpy(fileMemory, serialData.constData(), serialData.size());
        saveFile.unmap(fileMemory);
    }
    else
    {
        qWarning() << "ElaMetaType: saveToFile Map Failed!";
    }
    saveFile.flush();
    saveFile.close();
}

void ElaMetaType::saveToFile(const QString& fileName, const ElaVariant& variant)
{
    saveToFile(fileName, serialize(variant));
}

ElaVariant ElaMetaType::readFromFile(const QString& fileName, bool isByte)
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::ReadOnly | QIODevice::Text);
    uchar* fileMemory = saveFile.map(0, saveFile.size());
    if (isByte)
    {
        QByteArray serialData;
        if (fileMemory)
        {
            serialData = QByteArray(reinterpret_cast<const char*>(fileMemory), saveFile.size());
            saveFile.unmap(fileMemory);
        }
        else
        {
            serialData = saveFile.readAll();
        }
        saveFile.close();
        return deserializeFromByte(serialData);
    }
    else
    {
        QString serialData;
        if (fileMemory)
        {
            serialData = QString::fromUtf8(reinterpret_cast<const char*>(fileMemory), saveFile.size());
            saveFile.unmap(fileMemory);
        }
        else
        {
            serialData = saveFile.readAll();
        }
        saveFile.close();
        return deserialize(serialData);
    }
}

void ElaMetaType::registerMetaType(const QString& metaTypeName, ElaMetaTypeHash metaTypeHash, const QList<ElaMetaTypeId>& metaTypeIdList, std::function<void*()> constructorFunc, std::function<void(void*)> destructorFunc, std::function<std::shared_ptr<void>(void*)> constructorStdSharedFunc)
{
    Q_D(ElaMetaType);
    ElaMetaTypeInfo metaTypeInfo;
    metaTypeInfo._metaTypeIdFunc = [=](ElaVariant::MetaFlag metaFlag) -> ElaMetaTypeId {
        if (metaFlag < 0 || metaFlag >= metaTypeIdList.count())
        {
            return QMetaType::UnknownType;
        }
        return metaTypeIdList[metaFlag];
    };

    d->_allTypeHashMap.insert(metaTypeHash, metaTypeName);
    metaTypeInfo._metaTypeName = metaTypeName;
    metaTypeInfo._metaTypeHash = metaTypeHash;
    metaTypeInfo._isValid = true;
    metaTypeInfo._constructorFunc = std::move(constructorFunc);
    metaTypeInfo._destructorFunc = std::move(destructorFunc);
    metaTypeInfo._constructorStdSharedFunc = std::move(constructorStdSharedFunc);
    d->_metaTypeMap.insert(metaTypeHash, metaTypeInfo);
}

void ElaMetaType::registerMetaEnum(const QString& metaEnumName, ElaMetaTypeHash metaTypeHash)
{
    Q_D(ElaMetaType);
    d->_allTypeHashMap.insert(metaTypeHash, metaEnumName);
    d->_enumTypeHashMap.insert(metaTypeHash, metaEnumName);
}

void ElaMetaType::registerMetaField(ElaMetaTypeHash metaTypeHash, const ElaMetaField& field)
{
    Q_D(ElaMetaType);
    if (!d->_metaTypeMap.contains(metaTypeHash))
    {
        return;
    }
    auto& metaTypeInfo = d->_metaTypeMap[metaTypeHash];
    if (metaTypeInfo._fieldList.contains(field))
    {
        qDebug() << QString("ElaMetaType: Field %1 Duplication!").arg(field._fieldName);
        return;
    }
    metaTypeInfo._fieldList.append(field);
}

void ElaMetaType::registerParentMetaType(ElaMetaTypeHash metaTypeHash, ElaMetaTypeHash parentMetaTypeHash, const std::shared_ptr<ElaMetaFieldCaster>& fieldCaster)
{
    Q_D(ElaMetaType);
    if (!d->_metaTypeMap.contains(metaTypeHash) || !d->_metaTypeMap.contains(parentMetaTypeHash))
    {
        return;
    }
    auto& metaTypeInfo = d->_metaTypeMap[metaTypeHash];
    auto parentMetaTypeInfo = d->_metaTypeMap[parentMetaTypeHash];
    metaTypeInfo._parentMetaTypeHashList.append(parentMetaTypeHash);
    auto& fieldList = parentMetaTypeInfo._fieldList;
    if (fieldList.isEmpty())
    {
        return;
    }
    for (auto& field: fieldList)
    {
        // 菱形继承过滤
        if (!metaTypeInfo._fieldList.contains(field))
        {
            field._casterList.prepend(fieldCaster);
            metaTypeInfo._fieldList.append(field);
        }
    }
}

bool ElaMetaType::isMetaTypeRegistered(ElaMetaTypeHash metaTypeHash) const
{
    Q_D(const ElaMetaType);
    return d->_isMetaTypeRegistered(metaTypeHash);
}
