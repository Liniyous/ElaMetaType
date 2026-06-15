#include "ElaMetaTypePrivate.h"
#include "ElaVariant.h"
#include <QByteArray>
#include <QElapsedTimer>
#include <QPixmap>
#include <QPointF>
#include <QRect>
#include <QSize>
#include <QTimeZone>

ElaMetaTypePrivate::ElaMetaTypePrivate(QObject* parent)
    : QObject(parent), q_ptr(nullptr)
{
}

ElaMetaTypePrivate::~ElaMetaTypePrivate() = default;

inline const ElaMetaTypeInfo& ElaMetaTypePrivate::_getMetaTypeInfo(ElaMetaTypeHash metaTypeHash)
{
    auto it = _metaTypeMap.find(metaTypeHash);
    if (it == _metaTypeMap.end())
    {
        return _invalidMetaTypeInfo;
    }
    return *it;
}

QString ElaMetaTypePrivate::_getMetaTypePath(ElaMetaTypeHash metaTypeHash)
{
    const auto& metaTypeInfo = _getMetaTypeInfo(metaTypeHash);
    if (metaTypeInfo._isValid)
    {
        QString parentMetaTypePath;
        for (auto& parentMetaHash: metaTypeInfo._parentMetaTypeHashList)
        {
            parentMetaTypePath.append(_getMetaTypePath(parentMetaHash) + "/");
        }
        if (parentMetaTypePath.isEmpty())
        {
            return metaTypeInfo._metaTypeName;
        }
        parentMetaTypePath.remove(parentMetaTypePath.size() - 1, 1);
        return metaTypeInfo._metaTypeName + "/" + parentMetaTypePath;
    }
    return {};
}

bool ElaMetaTypePrivate::_isMetaTypeRegistered(ElaMetaTypeHash metaTypeHash) const
{
    return _allTypeHashMap.contains(metaTypeHash);
}

bool ElaMetaTypePrivate::_isEnumTypeRegistered(ElaMetaTypeHash metaTypeHash) const
{
    return _enumTypeHashMap.contains(metaTypeHash);
}

void ElaMetaTypePrivate::_serializeMetaType(const ElaMetaTypeInfo& metaTypeInfo, const ElaVariant& metaVariant, pugi::xml_node& metaNode)
{
    if (metaTypeInfo._isValid)
    {
        ElaVariant::MetaFlag metaFlag = metaVariant.getMetaFlag();
        metaNode.append_attribute("Hash").set_value(metaTypeInfo._metaTypeHash);
        metaNode.append_attribute("Type").set_value(_getMetaTypePath(metaTypeInfo._metaTypeHash).toUtf8().data());
        metaNode.append_attribute("Flag").set_value(metaFlag);
        const auto& fieldList = metaTypeInfo._fieldList;
        for (const auto& field: fieldList)
        {
            auto data = metaVariant.constData();
            if (metaFlag == ElaVariant::MetaFlag::Heap)
            {
                data = *static_cast<void* const*>(data);
            }
            else if (metaFlag == ElaVariant::MetaFlag::StdSharedPointer)
            {
                data = static_cast<const std::shared_ptr<void>*>(data)->get();
            }
            const void* tempData = data;
            for (const auto& caster: field._casterList)
            {
                tempData = caster->cast(tempData);
            }
            auto fieldVariant = field._operator->getValue(tempData);
            const QString& fieldName = field._fieldName;
            const QString& fieldDesc = field._fieldDesc;
            auto fieldNode = metaNode.append_child("Field");
            fieldNode.append_attribute("Field").set_value(fieldName.toUtf8().data());
            if (metaTypeInfo._metaTypeName != field._className)
            {
                fieldNode.append_attribute("FromType").set_value(field._className.toUtf8().data());
            }
            fieldNode.append_attribute("Desc").set_value(fieldDesc.toUtf8().data());
            _serializeElement(fieldVariant, fieldNode);
        }
    }
    else
    {
        // 基础类型序列化 只发生一次
        _serializeElement(metaVariant, metaNode);
    }
}

void ElaMetaTypePrivate::_serializeElement(const ElaVariant& itemVariant, pugi::xml_node& elementNode)
{
    QString typeName = itemVariant.getTypeName();
    ElaMetaTypeHash metaHash = itemVariant.getMetaHash();
    ElaVariant::MetaFlag metaFlag = itemVariant.getMetaFlag();
    switch (metaHash)
    {
    case getMetaTypeHash<bool>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toBool() ? "true" : "false");
        break;
    }
    case getMetaTypeHash<short>():
    case getMetaTypeHash<int>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toInt());
        break;
    }
    case getMetaTypeHash<long>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.value<long>());
        break;
    }
    case getMetaTypeHash<ushort>():
    case getMetaTypeHash<uint>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toUInt());
        break;
    }
    case getMetaTypeHash<ulong>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.value<ulong>());
        break;
    }
    case getMetaTypeHash<qlonglong>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toLongLong());
        break;
    }
    case getMetaTypeHash<qulonglong>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toULongLong());
        break;
    }
    case getMetaTypeHash<float>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toFloat());
        break;
    }
    case getMetaTypeHash<double>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toReal());
        break;
    }
    case getMetaTypeHash<QChar>():
    case getMetaTypeHash<signed char>():
    case getMetaTypeHash<uchar>():
    case getMetaTypeHash<char>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toChar().toLatin1());
        break;
    }
    case getMetaTypeHash<QString>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toString().toUtf8().data());
        break;
    }
    case getMetaTypeHash<QStringList>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.toStringList().join(",").toUtf8().data());
        break;
    }
    case getMetaTypeHash<QDate>():
    {
        QDate date = itemVariant.toDate();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3").arg(date.year()).arg(date.month()).arg(date.day()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QTime>():
    {
        QTime time = itemVariant.toTime();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3,%4").arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QDateTime>():
    {
        QDateTime dateTime = itemVariant.toDateTime();
        QDate date = dateTime.date();
        QTime time = dateTime.time();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3,%4,%5,%6,%7,%8").arg(date.year()).arg(date.month()).arg(date.day()).arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec()).arg(QString(dateTime.timeZone().id())).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QRect>():
    {
        QRect rect = itemVariant.toRect();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3,%4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QRectF>():
    {
        QRectF rect = itemVariant.toRectF();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3,%4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QSize>():
    {
        QSize size = itemVariant.toSize();
        elementNode.append_attribute("Value").set_value(QString("%1,%2").arg(size.width()).arg(size.height()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QSizeF>():
    {
        QSizeF size = itemVariant.toSizeF();
        elementNode.append_attribute("Value").set_value(QString("%1,%2").arg(size.width()).arg(size.height()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QLine>():
    {
        QLine line = itemVariant.toLine();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3,%4").arg(line.x1()).arg(line.y1()).arg(line.x2()).arg(line.y2()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QLineF>():
    {
        QLineF line = itemVariant.toLineF();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3,%4").arg(line.x1()).arg(line.y1()).arg(line.x2()).arg(line.y2()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QPoint>():
    {
        QPoint point = itemVariant.toPoint();
        elementNode.append_attribute("Value").set_value(QString("%1,%2").arg(point.x()).arg(point.y()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QPointF>():
    {
        QPointF point = itemVariant.toPointF();
        elementNode.append_attribute("Value").set_value(QString("%1,%2").arg(point.x()).arg(point.y()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QColor>():
    {
        auto color = itemVariant.value<QColor>();
        elementNode.append_attribute("Value").set_value(QString("%1,%2,%3,%4").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()).toUtf8().data());
        break;
    }
    case getMetaTypeHash<QByteArray>():
    {
        QByteArray bytes = itemVariant.toByteArray();
        elementNode.append_attribute("Value").set_value(bytes.data(), bytes.size());
        break;
    }
    case getMetaTypeHash<std::string>():
    {
        elementNode.append_attribute("Value").set_value(itemVariant.value<std::string>().data());
        break;
    }
    case getMetaTypeHash<ElaVariantPair>():
    {
        auto variantPair = itemVariant.value<QPair<ElaVariant, ElaVariant>>();
        auto firstNode = elementNode.append_child("First");
        _serializeElement(variantPair.first, firstNode);
        auto secondNode = elementNode.append_child("Second");
        _serializeElement(variantPair.second, secondNode);
        break;
    }
    case getMetaTypeHash<ElaVariantList>():
    {
        auto elaVariantList = itemVariant.value<ElaVariantList>();
        elementNode.append_attribute("Count").set_value(elaVariantList.count());
        for (const auto& variant: elaVariantList)
        {
            auto node = elementNode.append_child("Item");
            _serializeElement(variant, node);
        }
        break;
    }
    default:
    {
        // 处理自定义类型和枚举
        const auto& metaTypeInfo = _getMetaTypeInfo(metaHash);
        if (metaTypeInfo._isValid)
        {
            _serializeMetaType(metaTypeInfo, itemVariant, elementNode);
        }
        else
        {
            if (_isEnumTypeRegistered(metaHash))
            {
                // 视为Int处理枚举
                elementNode.append_attribute("Hash").set_value(metaHash);
                elementNode.append_attribute("Flag").set_value(metaFlag);
                elementNode.append_attribute("Value").set_value(itemVariant.toInt());
            }
            else
            {
                qDebug() << "ElaMetaType: Unsupported MetaType:" << itemVariant.getTypeName() << metaHash;
            }
        }
        return;
    }
    }
    elementNode.append_attribute("Type").set_value(typeName.toUtf8().data());
    elementNode.append_attribute("Hash").set_value(metaHash);
    elementNode.append_attribute("Flag").set_value(metaFlag);
}

ElaVariant ElaMetaTypePrivate::_deserializeMetaType(pugi::xml_node& metaNode)
{
    QString typeName = QString::fromUtf8(metaNode.attribute("Type").as_string());
    ElaMetaTypeHash metaTypeHash = metaNode.attribute("Hash").as_uint();

    ElaVariant::MetaFlag flag = static_cast<ElaVariant::MetaFlag>(metaNode.attribute("Flag").as_int());
    const auto& metaTypeInfo = _getMetaTypeInfo(metaTypeHash);
    if (metaTypeInfo._isValid)
    {
        // 创建对象实例
        void* obj = metaTypeInfo._constructorFunc();
        if (!obj)
        {
            qDebug() << "ElaMetaType: Failed To Create Object For Type:" << typeName;
            return _invalidVariant;
        }
        auto fieldNodes = metaNode.children();
        int fieldIndex = 0;
        for (auto& fieldNode: fieldNodes)
        {
            if (fieldIndex >= metaTypeInfo._fieldList.count())
            {
                qDebug() << "ElaMetaType: Field Index Out Of Range! ";
                break;
            }
            const auto& metaField = metaTypeInfo._fieldList[fieldIndex++];
            QString fieldName = QString::fromUtf8(fieldNode.attribute("Field").as_string());
            if (fieldName != metaField._fieldName)
            {
                qDebug() << "ElaMetaType: Field Index Error! Current Field:" << fieldName << "Index Field:" << metaField._fieldName;
                continue;
            }
            ElaVariant fieldVariant = _deserializeElement(fieldNode);
            if (fieldVariant.isValid())
            {
                void* tempObj = obj;
                for (const auto& caster: metaField._casterList)
                {
                    tempObj = caster->cast(tempObj);
                }
                metaField._operator->setValue(tempObj, fieldVariant);
            }
        }
        ElaVariant result;
        if (flag == ElaVariant::MetaFlag::Stack)
        {
            result = ElaVariant(metaTypeHash, metaTypeInfo._metaTypeIdFunc(flag), obj, flag);
            // 清理对象
            metaTypeInfo._destructorFunc(obj);
        }
        else if (flag == ElaVariant::MetaFlag::Heap)
        {
            result = ElaVariant(metaTypeHash, metaTypeInfo._metaTypeIdFunc(flag), &obj, flag);
        }
        else if (flag == ElaVariant::MetaFlag::StdSharedPointer)
        {
            std::shared_ptr<void> sharedPtr = metaTypeInfo._constructorStdSharedFunc(obj);
            result = ElaVariant(metaTypeHash, metaTypeInfo._metaTypeIdFunc(flag), &sharedPtr, flag);
        }
        return result;
    }
    // 基本类型反序列化 只发生一次
    return _deserializeElement(metaNode);
}

ElaVariant ElaMetaTypePrivate::_deserializeElement(pugi::xml_node& elementNode)
{
    QString itemValue = QString::fromUtf8(elementNode.attribute("Value").as_string());
    ElaMetaTypeHash metaTypeHash = elementNode.attribute("Hash").as_uint();
    switch (metaTypeHash)
    {
    case getMetaTypeHash<bool>():
    {
        return ElaVariant::fromValue(itemValue == "true");
    }
    case getMetaTypeHash<short>():
    case getMetaTypeHash<int>():
    {
        return ElaVariant::fromValue(itemValue.toInt());
    }
    case getMetaTypeHash<long>():
    {
        return ElaVariant::fromValue(itemValue.toLong());
    }
    case getMetaTypeHash<ushort>():
    case getMetaTypeHash<uint>():
    {
        return ElaVariant::fromValue(itemValue.toUInt());
    }
    case getMetaTypeHash<ulong>():
    {
        return ElaVariant::fromValue(itemValue.toULong());
    }
    case getMetaTypeHash<qlonglong>():
    {
        return ElaVariant::fromValue(itemValue.toLongLong());
    }
    case getMetaTypeHash<qulonglong>():
    {
        return ElaVariant::fromValue(itemValue.toULongLong());
    }
    case getMetaTypeHash<float>():
    {
        return ElaVariant::fromValue(itemValue.toFloat());
    }
    case getMetaTypeHash<double>():
    {
        return ElaVariant::fromValue(itemValue.toDouble());
    }
    case getMetaTypeHash<QChar>():
    case getMetaTypeHash<uchar>():
    case getMetaTypeHash<signed char>():
    case getMetaTypeHash<char>():
    {
        return ElaVariant::fromValue(*itemValue.data());
    }
    case getMetaTypeHash<QString>():
    {
        return ElaVariant::fromValue(itemValue);
    }
    case getMetaTypeHash<QStringList>():
    {
        return ElaVariant::fromValue(itemValue.split(","));
    }
    case getMetaTypeHash<QDate>():
    {
        auto dateList = itemValue.split(",");
        if (dateList.count() != 3)
        {
            return {};
        }
        return ElaVariant::fromValue(QDate(dateList[0].toInt(), dateList[1].toInt(), dateList[2].toInt()));
    }
    case getMetaTypeHash<QTime>():
    {
        auto timeList = itemValue.split(",");
        if (timeList.count() != 4)
        {
            return {};
        }
        return ElaVariant::fromValue(QTime(timeList[0].toInt(), timeList[1].toInt(), timeList[2].toInt(), timeList[3].toInt()));
    }
    case getMetaTypeHash<QDateTime>():
    {
        auto dateTimeList = itemValue.split(",");
        if (dateTimeList.count() != 8)
        {
            return {};
        }
        return ElaVariant::fromValue(QDateTime(QDate(dateTimeList[0].toInt(), dateTimeList[1].toInt(), dateTimeList[2].toInt()), QTime(dateTimeList[3].toInt(), dateTimeList[4].toInt(), dateTimeList[5].toInt(), dateTimeList[6].toInt()), QTimeZone(dateTimeList[7].toUtf8())));
    }
    case getMetaTypeHash<QRect>():
    {
        auto rectList = itemValue.split(",");
        if (rectList.count() != 4)
        {
            return {};
        }
        return ElaVariant::fromValue(QRect(rectList[0].toInt(), rectList[1].toInt(), rectList[2].toInt(), rectList[3].toInt()));
    }
    case getMetaTypeHash<QRectF>():
    {
        auto rectList = itemValue.split(",");
        if (rectList.count() != 4)
        {
            return {};
        }
        return ElaVariant::fromValue(QRectF(rectList[0].toDouble(), rectList[1].toDouble(), rectList[2].toDouble(), rectList[3].toDouble()));
    }
    case getMetaTypeHash<QSize>():
    {
        auto sizeList = itemValue.split(",");
        if (sizeList.count() != 2)
        {
            return {};
        }
        return ElaVariant::fromValue(QSize(sizeList[0].toInt(), sizeList[1].toInt()));
    }
    case getMetaTypeHash<QSizeF>():
    {
        auto sizeList = itemValue.split(",");
        if (sizeList.count() != 2)
        {
            return {};
        }
        return ElaVariant::fromValue(QSizeF(sizeList[0].toDouble(), sizeList[1].toDouble()));
    }
    case getMetaTypeHash<QLine>():
    {
        auto lineList = itemValue.split(",");
        if (lineList.count() != 4)
        {
            return {};
        }
        return ElaVariant::fromValue(QLine(lineList[0].toInt(), lineList[1].toInt(), lineList[2].toInt(), lineList[3].toInt()));
    }
    case getMetaTypeHash<QLineF>():
    {
        auto lineList = itemValue.split(",");
        if (lineList.count() != 4)
        {
            return {};
        }
        return ElaVariant::fromValue(QLineF(lineList[0].toDouble(), lineList[1].toDouble(), lineList[2].toDouble(), lineList[3].toDouble()));
    }
    case getMetaTypeHash<QPoint>():
    {
        auto pointList = itemValue.split(",");
        if (pointList.count() != 2)
        {
            return {};
        }
        return ElaVariant::fromValue(QPoint(pointList[0].toInt(), pointList[1].toInt()));
    }
    case getMetaTypeHash<QPointF>():
    {
        auto pointList = itemValue.split(",");
        if (pointList.count() != 2)
        {
            return {};
        }
        return ElaVariant::fromValue(QPointF(pointList[0].toDouble(), pointList[1].toDouble()));
    }
    case getMetaTypeHash<QColor>():
    {
        auto colorList = itemValue.split(",");
        if (colorList.count() != 4)
        {
            return {};
        }
        return ElaVariant::fromValue(QColor(colorList[0].toInt(), colorList[1].toInt(), colorList[2].toInt(), colorList[3].toInt()));
    }
    case getMetaTypeHash<QByteArray>():
    {
        return ElaVariant::fromValue(itemValue);
    }
    case getMetaTypeHash<std::string>():
    {
        return ElaVariant::fromValue(itemValue.toStdString());
    }
    case getMetaTypeHash<ElaVariantPair>():
    {
        auto firstNode = elementNode.child("First");
        ElaVariant firstVariant = _deserializeElement(firstNode);
        auto secondNode = elementNode.child("Second");
        ElaVariant secondVariant = _deserializeElement(secondNode);
        return ElaVariant::fromValue(ElaVariantPair(firstVariant, secondVariant));
    }
    case getMetaTypeHash<ElaVariantList>():
    {
        ElaVariantList variantList;
        auto childNodes = elementNode.children();
        for (auto childNode: childNodes)
        {
            ElaVariant childItemValue = _deserializeElement(childNode);
            if (childItemValue.isValid())
            {
                variantList.append(childItemValue);
            }
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &variantList, ElaVariant::MetaFlag::Stack};
    }
    default:
    {
        if (_metaTypeMap.contains(metaTypeHash))
        {
            return _deserializeMetaType(elementNode);
        }
        if (_isEnumTypeRegistered(metaTypeHash))
        {
            return ElaVariant::fromValue(itemValue.toUInt());
        }
    }
    }
    qDebug() << "ElaMetaType: Unsupported Element Item Hash:" << metaTypeHash;
    return {};
}

void ElaMetaTypePrivate::_serializeMetaType(const ElaMetaTypeInfo& metaTypeInfo, const ElaVariant& metaVariant, ElaMetaBuffer& metaBuffer)
{
    ElaVariant::MetaFlag metaFlag = metaVariant.getMetaFlag();
    if (metaTypeInfo._isValid)
    {
        const auto& fieldList = metaTypeInfo._fieldList;
        // 类型哈希、Flag、FiledCount
        metaBuffer.writeValue(metaTypeInfo._metaTypeHash);
        metaBuffer.writeValue(static_cast<unsigned char>(metaFlag));
        metaBuffer.writeValue(static_cast<unsigned char>(fieldList.count()));
        for (const auto& field: fieldList)
        {
            auto data = metaVariant.constData();
            if (metaFlag == ElaVariant::MetaFlag::Heap)
            {
                data = *static_cast<void* const*>(data);
            }
            else if (metaFlag == ElaVariant::MetaFlag::StdSharedPointer)
            {
                data = static_cast<const std::shared_ptr<void>*>(data)->get();
            }
            for (const auto& caster: field._casterList)
            {
                data = caster->cast(data);
            }
            auto fieldVariant = field._operator->getValue(data);
            _serializeElement(fieldVariant, metaBuffer);
        }
    }
    else
    {
        // 基础类型序列化 只发生一次
        _serializeElement(metaVariant, metaBuffer);
    }
}

void ElaMetaTypePrivate::_serializeElement(const ElaVariant& itemVariant, ElaMetaBuffer& metaBuffer)
{
    ElaMetaTypeHash metaHash = itemVariant.getMetaHash();
    metaBuffer.writeValue(metaHash);
    switch (metaHash)
    {
    case getMetaTypeHash<bool>():
    {
        metaBuffer.writeValue(itemVariant.toBool());
        break;
    }
    case getMetaTypeHash<short>():
    case getMetaTypeHash<int>():
    {
        metaBuffer.writeValue(itemVariant.toInt());
        break;
    }
    case getMetaTypeHash<long>():
    {
        metaBuffer.writeValue(itemVariant.value<quint64>());
        break;
    }
    case getMetaTypeHash<ushort>():
    case getMetaTypeHash<uint>():
    {
        metaBuffer.writeValue(itemVariant.toUInt());
        break;
    }
    case getMetaTypeHash<ulong>():
    {
        metaBuffer.writeValue(itemVariant.value<ulong>());
        break;
    }
    case getMetaTypeHash<qlonglong>():
    {
        metaBuffer.writeValue(itemVariant.toLongLong());
        break;
    }
    case getMetaTypeHash<qulonglong>():
    {
        metaBuffer.writeValue(itemVariant.toULongLong());
        break;
    }
    case getMetaTypeHash<float>():
    {
        metaBuffer.writeValue(itemVariant.toFloat());
        break;
    }
    case getMetaTypeHash<double>():
    {
        metaBuffer.writeValue(itemVariant.toReal());
        break;
    }
    case getMetaTypeHash<QChar>():
    case getMetaTypeHash<signed char>():
    case getMetaTypeHash<uchar>():
    case getMetaTypeHash<char>():
    {
        metaBuffer.writeValue(itemVariant.toChar().toLatin1());
        break;
    }
    case getMetaTypeHash<QString>():
    {
        QByteArray stringData = itemVariant.toString().toUtf8();
        int stringSize = static_cast<int>(stringData.size());
        metaBuffer.writeValue(stringSize);
        metaBuffer.writeRawData(stringData.constData(), stringSize);
        break;
    }
    case getMetaTypeHash<QStringList>():
    {
        QStringList stringList = itemVariant.toStringList();
        metaBuffer.writeValue(static_cast<int>(stringList.size()));
        for (auto& string: stringList)
        {
            QByteArray stringData = string.toUtf8();
            int stringSize = static_cast<int>(stringData.size());
            metaBuffer.writeValue(stringSize);
            metaBuffer.writeRawData(stringData.constData(), stringSize);
        }
        break;
    }
    case getMetaTypeHash<QDate>():
    {
        QDate date = itemVariant.toDate();
        metaBuffer.writeValue(date.year());
        metaBuffer.writeValue(date.month());
        metaBuffer.writeValue(date.day());
        break;
    }
    case getMetaTypeHash<QTime>():
    {
        QTime time = itemVariant.toTime();
        metaBuffer.writeValue(time.hour());
        metaBuffer.writeValue(time.minute());
        metaBuffer.writeValue(time.second());
        metaBuffer.writeValue(time.msec());
        break;
    }
    case getMetaTypeHash<QDateTime>():
    {
        QDateTime dateTime = itemVariant.toDateTime();
        QDate date = dateTime.date();
        QTime time = dateTime.time();
        metaBuffer.writeValue(date.year());
        metaBuffer.writeValue(date.month());
        metaBuffer.writeValue(date.day());
        metaBuffer.writeValue(time.hour());
        metaBuffer.writeValue(time.minute());
        metaBuffer.writeValue(time.second());
        metaBuffer.writeValue(time.msec());
        auto timeZoneData = QString(dateTime.timeZone().id()).toUtf8();
        metaBuffer.writeValue(timeZoneData.size());
        metaBuffer.writeRawData(timeZoneData.constData(), timeZoneData.size());
        break;
    }
    case getMetaTypeHash<QRect>():
    {
        QRect rect = itemVariant.toRect();
        metaBuffer.writeValue(rect.x());
        metaBuffer.writeValue(rect.y());
        metaBuffer.writeValue(rect.width());
        metaBuffer.writeValue(rect.height());
        break;
    }
    case getMetaTypeHash<QRectF>():
    {
        QRectF rect = itemVariant.toRectF();
        metaBuffer.writeValue(rect.x());
        metaBuffer.writeValue(rect.y());
        metaBuffer.writeValue(rect.width());
        metaBuffer.writeValue(rect.height());
        break;
    }
    case getMetaTypeHash<QSize>():
    {
        QSize size = itemVariant.toSize();
        metaBuffer.writeValue(size.width());
        metaBuffer.writeValue(size.height());
        break;
    }
    case getMetaTypeHash<QSizeF>():
    {
        QSizeF size = itemVariant.toSizeF();
        metaBuffer.writeValue(size.width());
        metaBuffer.writeValue(size.height());
        break;
    }
    case getMetaTypeHash<QLine>():
    {
        QLine line = itemVariant.toLine();
        metaBuffer.writeValue(line.x1());
        metaBuffer.writeValue(line.y1());
        metaBuffer.writeValue(line.x2());
        metaBuffer.writeValue(line.y2());
        break;
    }
    case getMetaTypeHash<QLineF>():
    {
        QLineF line = itemVariant.toLineF();
        metaBuffer.writeValue(line.x1());
        metaBuffer.writeValue(line.y1());
        metaBuffer.writeValue(line.x2());
        metaBuffer.writeValue(line.y2());
        break;
    }
    case getMetaTypeHash<QPoint>():
    {
        QPoint point = itemVariant.toPoint();
        metaBuffer.writeValue(point.x());
        metaBuffer.writeValue(point.y());
        break;
    }
    case getMetaTypeHash<QPointF>():
    {
        QPointF point = itemVariant.toPointF();
        metaBuffer.writeValue(point.x());
        metaBuffer.writeValue(point.y());
        break;
    }
    case getMetaTypeHash<QColor>():
    {
        auto color = itemVariant.value<QColor>();
        metaBuffer.writeValue(color.red());
        metaBuffer.writeValue(color.green());
        metaBuffer.writeValue(color.blue());
        metaBuffer.writeValue(color.alpha());
        break;
    }
    case getMetaTypeHash<QByteArray>():
    {
        QByteArray bytes = itemVariant.toByteArray();
        metaBuffer.writeValue(static_cast<int>(bytes.size()));
        metaBuffer.writeRawData(bytes.constData(), static_cast<int>(bytes.size()));
        break;
    }
    case getMetaTypeHash<QPixmap>():
    {
        QImage image = itemVariant.value<QPixmap>().toImage();
        metaBuffer.writeValue(image.width());
        metaBuffer.writeValue(image.height());
        metaBuffer.writeValue(static_cast<unsigned char>(image.format()));
        int imageSizeInBytes = static_cast<int>(image.sizeInBytes());
        metaBuffer.writeValue(imageSizeInBytes);
        metaBuffer.writeRawData(reinterpret_cast<const char*>(image.bits()), imageSizeInBytes);
        break;
    }
    case getMetaTypeHash<QImage>():
    {
        auto image = itemVariant.value<QImage>();
        metaBuffer.writeValue(image.width());
        metaBuffer.writeValue(image.height());
        metaBuffer.writeValue(static_cast<unsigned char>(image.format()));
        int imageSizeInBytes = static_cast<int>(image.sizeInBytes());
        metaBuffer.writeValue(imageSizeInBytes);
        metaBuffer.writeRawData(reinterpret_cast<const char*>(image.bits()), imageSizeInBytes);
        break;
    }
    case getMetaTypeHash<std::string>():
    {
        auto string = itemVariant.value<std::string>();
        int stringSize = static_cast<int>(string.size());
        metaBuffer.writeValue(stringSize);
        metaBuffer.writeRawData(string.data(), stringSize);
        break;
    }
    case getMetaTypeHash<ElaVariantPair>():
    {
        auto variantPair = itemVariant.value<QPair<ElaVariant, ElaVariant>>();
        _serializeElement(variantPair.first, metaBuffer);
        _serializeElement(variantPair.second, metaBuffer);
        break;
    }
    case getMetaTypeHash<ElaVariantList>():
    {
        auto elaVariantList = itemVariant.value<ElaVariantList>();
        metaBuffer.writeValue(static_cast<int>(elaVariantList.count()));
        for (const auto& variant: elaVariantList)
        {
            _serializeElement(variant, metaBuffer);
        }
        break;
    }
    default:
    {
        // 处理自定义类型和枚举
        const auto& metaTypeInfo = _getMetaTypeInfo(metaHash);
        if (metaTypeInfo._isValid)
        {
            _serializeMetaType(metaTypeInfo, itemVariant, metaBuffer);
        }
        else
        {
            if (_isEnumTypeRegistered(metaHash))
            {
                // 视为Int处理枚举
                metaBuffer.writeValue(itemVariant.toInt());
            }
            else
            {
                qDebug() << "ElaMetaType: Unsupported MetaType:" << itemVariant.getTypeName() << metaHash;
            }
        }
        break;
    }
    }
}

ElaVariant ElaMetaTypePrivate::_deserializeMetaType(ElaMetaBuffer& metaBuffer)
{
    ElaMetaTypeHash metaTypeHash = 0;
    metaBuffer.readValue(metaTypeHash);

    unsigned char ucharFlag;
    metaBuffer.readValue(ucharFlag);
    auto flag = static_cast<ElaVariant::MetaFlag>(ucharFlag);

    unsigned char ucharFieldCount;
    int fliedCount = 0;
    metaBuffer.readValue(ucharFieldCount);
    fliedCount = static_cast<int>(ucharFieldCount);

    const auto& metaTypeInfo = _getMetaTypeInfo(metaTypeHash);
    if (metaTypeInfo._isValid)
    {
        // 创建对象实例
        void* obj = metaTypeInfo._constructorFunc();
        if (!obj)
        {
            qDebug() << "ElaMetaType: Failed To Create Object For TypeHash:" << metaTypeHash;
            return _invalidVariant;
        }

        int fieldListCount = metaTypeInfo._fieldList.count();
        for (int i = 0; i < fliedCount; i++)
        {
            if (i >= fieldListCount)
            {
                qDebug() << "ElaMetaType: Field Index Error! " << i;
                break;
            }
            ElaVariant fieldVariant = _deserializeElement(metaBuffer);
            if (fieldVariant.isValid())
            {
                const auto& metaField = metaTypeInfo._fieldList[i];
                void* tempObj = obj; // 保存原始指针的副本
                for (const auto& caster: metaField._casterList)
                {
                    tempObj = caster->cast(tempObj);
                }
                metaField._operator->setValue(tempObj, fieldVariant);
            }
        }
        switch (flag)
        {
        case ElaVariant::MetaFlag::Stack:
        {
            ElaVariant result = ElaVariant(metaTypeHash, metaTypeInfo._metaTypeIdFunc(flag), obj, flag);
            // 清理对象
            metaTypeInfo._destructorFunc(obj);
            return result;
        }
        case ElaVariant::MetaFlag::Heap:
        {
            return {metaTypeHash, metaTypeInfo._metaTypeIdFunc(flag), &obj, flag};
        }
        case ElaVariant::MetaFlag::StdSharedPointer:
        {
            std::shared_ptr<void> sharedPtr = metaTypeInfo._constructorStdSharedFunc(obj);
            return {metaTypeHash, metaTypeInfo._metaTypeIdFunc(flag), &sharedPtr, flag};
        }
        default:
        {
            break;
        }
        }
        return _invalidVariant;
    }
    // 基本类型反序列化 只发生一次
    return _deserializeElement(metaBuffer);
}

ElaVariant ElaMetaTypePrivate::_deserializeElement(ElaMetaBuffer& metaBuffer)
{
    ElaMetaTypeHash metaTypeHash;
    metaBuffer.readValue(metaTypeHash);

    int dataLength;
    thread_local QByteArray dataBuffer;
    thread_local QByteArray stringDataBuffer;
    int tempInt0, tempInt1, tempInt2, tempInt3;
    qreal tempDouble0, tempDouble1, tempDouble2, tempDouble3;
    switch (metaTypeHash)
    {
    case getMetaTypeHash<bool>():
    {
        bool value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<short>():
    case getMetaTypeHash<int>():
    {
        metaBuffer.readValue(tempInt0);
        return ElaVariant::fromValue(tempInt0);
    }
    case getMetaTypeHash<long>():
    {
        quint64 value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<ushort>():
    case getMetaTypeHash<uint>():
    {
        unsigned int value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<ulong>():
    {
        ulong value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<qlonglong>():
    {
        long long value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<qulonglong>():
    {
        unsigned long long value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<float>():
    {
        float value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<double>():
    {
        metaBuffer.readValue(tempDouble0);
        return ElaVariant::fromValue(tempDouble0);
    }
    case getMetaTypeHash<QChar>():
    case getMetaTypeHash<uchar>():
    {
        unsigned char value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<signed char>():
    case getMetaTypeHash<char>():
    {
        char value;
        metaBuffer.readValue(value);
        return ElaVariant::fromValue(value);
    }
    case getMetaTypeHash<QString>():
    {
        metaBuffer.readValue(dataLength);
        stringDataBuffer.resize(dataLength);
        metaBuffer.readRawData(stringDataBuffer.data(), dataLength);
        return ElaVariant::fromValue<QString>(stringDataBuffer);
    }
    case getMetaTypeHash<QStringList>():
    {
        QStringList stringList;
        stringList.clear();
        metaBuffer.readValue(tempInt0);
        for (int i = 0; i < tempInt0; i++)
        {
            metaBuffer.readValue(dataLength);
            stringDataBuffer.resize(dataLength);
            metaBuffer.readRawData(stringDataBuffer.data(), dataLength);
            stringList.append(stringDataBuffer);
        }
        return ElaVariant::fromValue(stringList);
    }
    case getMetaTypeHash<QDate>():
    {
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(tempInt2);
        return ElaVariant::fromValue(QDate(tempInt0, tempInt1, tempInt2));
    }
    case getMetaTypeHash<QTime>():
    {
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(tempInt2);
        metaBuffer.readValue(tempInt3);
        return ElaVariant::fromValue(QTime(tempInt0, tempInt1, tempInt2, tempInt3));
    }
    case getMetaTypeHash<QDateTime>():
    {
        int tempInt4, tempInt5, tempInt6;
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(tempInt2);
        metaBuffer.readValue(tempInt3);
        metaBuffer.readValue(tempInt4);
        metaBuffer.readValue(tempInt5);
        metaBuffer.readValue(tempInt6);
        metaBuffer.readValue(dataLength);
        stringDataBuffer.resize(dataLength);
        metaBuffer.readRawData(stringDataBuffer.data(), dataLength);
        return ElaVariant::fromValue(QDateTime(QDate(tempInt0, tempInt1, tempInt2), QTime(tempInt3, tempInt4, tempInt5, tempInt6), QTimeZone(stringDataBuffer)));
    }
    case getMetaTypeHash<QRect>():
    {
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(tempInt2);
        metaBuffer.readValue(tempInt3);
        return ElaVariant::fromValue(QRect(tempInt0, tempInt1, tempInt2, tempInt3));
    }
    case getMetaTypeHash<QRectF>():
    {
        metaBuffer.readValue(tempDouble0);
        metaBuffer.readValue(tempDouble1);
        metaBuffer.readValue(tempDouble2);
        metaBuffer.readValue(tempDouble3);
        return ElaVariant::fromValue(QRectF(tempDouble0, tempDouble1, tempDouble2, tempDouble3));
    }
    case getMetaTypeHash<QSize>():
    {
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        return ElaVariant::fromValue(QSize(tempInt0, tempInt1));
    }
    case getMetaTypeHash<QSizeF>():
    {
        metaBuffer.readValue(tempDouble0);
        metaBuffer.readValue(tempDouble1);
        return ElaVariant::fromValue(QSizeF(tempDouble0, tempDouble1));
    }
    case getMetaTypeHash<QLine>():
    {
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(tempInt2);
        metaBuffer.readValue(tempInt3);
        return ElaVariant::fromValue(QLine(tempInt0, tempInt1, tempInt2, tempInt3));
    }
    case getMetaTypeHash<QLineF>():
    {
        metaBuffer.readValue(tempDouble0);
        metaBuffer.readValue(tempDouble1);
        metaBuffer.readValue(tempDouble2);
        metaBuffer.readValue(tempDouble3);
        return ElaVariant::fromValue(QLineF(tempDouble0, tempDouble1, tempDouble2, tempDouble3));
    }
    case getMetaTypeHash<QPoint>():
    {
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        return ElaVariant::fromValue(QPoint(tempInt0, tempInt1));
    }
    case getMetaTypeHash<QPointF>():
    {
        metaBuffer.readValue(tempDouble0);
        metaBuffer.readValue(tempDouble1);
        return ElaVariant::fromValue(QPointF(tempDouble0, tempDouble1));
    }
    case getMetaTypeHash<QColor>():
    {
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(tempInt2);
        metaBuffer.readValue(tempInt3);
        return ElaVariant::fromValue(QColor(tempInt0, tempInt1, tempInt2, tempInt3));
    }
    case getMetaTypeHash<QByteArray>():
    {
        metaBuffer.readValue(dataLength);
        dataBuffer.resize(dataLength);
        metaBuffer.readRawData(dataBuffer.data(), dataLength);
        return ElaVariant::fromValue(dataBuffer);
    }
    case getMetaTypeHash<QPixmap>():
    {
        unsigned char imageFormat;
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(imageFormat);
        metaBuffer.readValue(dataLength);
        dataBuffer.resize(dataLength);
        metaBuffer.readRawData(dataBuffer.data(), dataLength);
        return ElaVariant::fromValue(QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(dataBuffer.data()), tempInt0, tempInt1, static_cast<QImage::Format>(imageFormat))).copy());
    }
    case getMetaTypeHash<QImage>():
    {
        unsigned char imageFormat;
        metaBuffer.readValue(tempInt0);
        metaBuffer.readValue(tempInt1);
        metaBuffer.readValue(imageFormat);
        metaBuffer.readValue(dataLength);
        dataBuffer.resize(dataLength);
        metaBuffer.readRawData(dataBuffer.data(), dataLength);
        return ElaVariant::fromValue(QImage(reinterpret_cast<uchar*>(dataBuffer.data()), tempInt0, tempInt1, static_cast<QImage::Format>(imageFormat)).copy());
    }
    case getMetaTypeHash<std::string>():
    {
        metaBuffer.readValue(dataLength);
        stringDataBuffer.resize(dataLength);
        metaBuffer.readRawData(stringDataBuffer.data(), dataLength);
        return ElaVariant::fromValue(std::string(stringDataBuffer.constData(), dataLength));
    }
    case getMetaTypeHash<ElaVariantPair>():
    {
        ElaVariant firstVariant = _deserializeElement(metaBuffer);
        ElaVariant secondVariant = _deserializeElement(metaBuffer);
        return ElaVariant::fromValue(ElaVariantPair(firstVariant, secondVariant));
    }
    case getMetaTypeHash<ElaVariantList>():
    {
        ElaVariantList variantList;
        int arrayCount = 0;
        metaBuffer.readValue(arrayCount);
        variantList.reserve(arrayCount);
        for (int i = 0; i < arrayCount; i++)
        {
            ElaVariant childItemValue = _deserializeElement(metaBuffer);
            if (childItemValue.isValid())
            {
                variantList.append(childItemValue);
            }
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &variantList, ElaVariant::MetaFlag::Stack};
    }
    default:
    {
        if (_metaTypeMap.contains(metaTypeHash))
        {
            return _deserializeMetaType(metaBuffer);
        }
        if (_isEnumTypeRegistered(metaTypeHash))
        {
            metaBuffer.readValue(tempInt0);
            return ElaVariant::fromValue(tempInt0);
        }
        break;
    }
    }
    qDebug() << "ElaMetaType: Unsupported Element Item Hash:" << metaTypeHash;
    return _invalidVariant;
}
