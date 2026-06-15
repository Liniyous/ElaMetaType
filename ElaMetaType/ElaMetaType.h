#ifndef ELAMETATYPE_H_
#define ELAMETATYPE_H_

#include "ElaMetaCollection.h"
#include "ElaMetaField.h"
#include "ElaMetaTypeMacro.h"
#include "ElaMetaTypeExport.h"
#include "ElaProperty.h"
#include "ElaSingleton.h"

#include <QList>
#include <QMetaMethod>
#include <QObject>
#include <memory>
#include <type_traits>
#include <utility>

class ElaMetaTypePrivate;
class ELA_METATYPE_EXPORT ElaMetaType : public QObject
{
    Q_OBJECT
    Q_Q_CREATE(ElaMetaType)
    Q_SINGLETON_CREATE_H(ElaMetaType)

public:
    // 可读 速度稍慢
    QString serialize(const ElaVariant& variant);
    ElaVariant deserialize(const QString& serialData);

    template <typename T>
    QString serialize(const T& variant)
    {
        return serialize(ElaVariant::fromValue(variant));
    }

    template <typename T>
    T deserialize(const QString& serialData)
    {
        return deserialize(serialData).value<T>();
    }

    // 不可读 速度快
    QByteArray serializeToByte(const ElaVariant& variant, bool isCompress = false);
    ElaVariant deserializeFromByte(QByteArray& serialData);

    template <typename T, typename = typename std::enable_if<!std::is_same<typename std::decay<T>::type, ElaVariant>::value>::type>
    QByteArray serializeToByte(const T& variant, bool isCompress = false)
    {
        return serializeToByte(ElaVariant::fromValue(variant), isCompress);
    }

    template <typename T, typename = typename std::enable_if<!std::is_same<typename std::decay<T>::type, ElaVariant>::value>::type>
    T deserializeFromByte(QByteArray& serialData)
    {
        return deserializeFromByte(serialData).value<T>();
    }

    static void saveToFile(const QString& fileName, const QString& serialData);
    static void saveToFile(const QString& fileName, const QByteArray& serialData);
    void saveToFile(const QString& fileName, const ElaVariant& variant);
    ElaVariant readFromFile(const QString& fileName, bool isByte = false);

    void registerMetaType(const QString& metaTypeName, ElaMetaTypeHash metaTypeHash, const QList<ElaMetaTypeId>& metaTypeIdList, std::function<void*()> constructorFunc, std::function<void(void*)> destructorFunc, std::function<std::shared_ptr<void>(void*)> constructorStdSharedFunc);
    void registerMetaEnum(const QString& metaEnumName, ElaMetaTypeHash metaTypeHash);
    void registerMetaField(ElaMetaTypeHash metaTypeHash, const ElaMetaField& field);
    void registerParentMetaType(ElaMetaTypeHash metaTypeHash, ElaMetaTypeHash parentMetaTypeHash, const std::shared_ptr<ElaMetaFieldCaster>& fieldCaster);

    Q_REQUIRED_RESULT bool isMetaTypeRegistered(ElaMetaTypeHash metaTypeHash) const;

private:
    explicit ElaMetaType(QObject* parent = Q_NULLPTR);
    ~ElaMetaType() override;
};

#endif //ELAMETATYPE_H_
