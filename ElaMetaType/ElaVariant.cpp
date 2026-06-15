#include "ElaVariant.h"

ElaVariant::ElaVariant()
    : QVariant()
{
}

ElaVariant::ElaVariant(const ElaVariant& other)
    : QVariant(other)
{
    _typeName = other._typeName;
    _metaFlag = other._metaFlag;
    _metaHash = other._metaHash;
}

ElaVariant::ElaVariant(ElaMetaTypeHash metaHash, int typeId, const void* copy, ElaVariant::MetaFlag flags)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    : QVariant(typeId, copy, flags == MetaFlag::Heap)
#else
    : QVariant(QMetaType(typeId), copy)
#endif
{
    _metaFlag = flags;
    _metaHash = metaHash;
}

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
ElaVariant::ElaVariant(const std::string& string)
    : QVariant()
{
    setValue(string);
    _typeName = "std::string";
    _metaHash = getMetaTypeHash<std::string>();
}

ElaVariant::ElaVariant(const QStringList& qStringList)
    : QVariant(qStringList)
{
    _metaHash = getMetaTypeHash<QStringList>();
}
#endif

QString ElaVariant::getTypeName() const
{
    if (_typeName.isEmpty())
    {
        return QVariant::typeName();
    }
    return _typeName;
}

ElaVariant::MetaFlag ElaVariant::getMetaFlag() const
{
    return _metaFlag;
}

ElaMetaTypeHash ElaVariant::getMetaHash() const
{
    return _metaHash;
}
