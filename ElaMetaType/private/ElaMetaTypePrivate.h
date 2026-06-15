#ifndef ELAMETATYPE_ELAMETATYPEPRIVATE_H
#define ELAMETATYPE_ELAMETATYPEPRIVATE_H
#include "ElaMetaBuffer.h"
#include "ElaMetaField.h"
#include "ElaMetaTypeExport.h"
#include "ElaProperty.h"
#include "ElaVariant.h"
#include "pugixml.h"
#include <QDebug>
#include <QList>
#include <QMetaMethod>
#include <QObject>

struct ElaMetaTypeInfo {
    std::function<ElaMetaTypeId(ElaVariant::MetaFlag metaFlag)> _metaTypeIdFunc{nullptr};
    QString _metaTypeName;
    ElaMetaTypeHash _metaTypeHash{0};
    QList<ElaMetaTypeHash> _parentMetaTypeHashList;
    std::function<void*()> _constructorFunc{nullptr};
    std::function<std::shared_ptr<void>(void*)> _constructorStdSharedFunc{nullptr};
    std::function<void(void*)> _destructorFunc{nullptr};
    QVector<ElaMetaField> _fieldList;
    bool _isValid{false};
};

class ElaMetaType;
class ElaMetaTypePrivate : public QObject
{
    Q_OBJECT
    Q_D_CREATE(ElaMetaType)
public:
    explicit ElaMetaTypePrivate(QObject* parent = nullptr);
    ~ElaMetaTypePrivate() override;

private:
    ElaVariant _invalidVariant;
    QString _version{"1.0.0"};
    ElaMetaTypeInfo _invalidMetaTypeInfo;
    QHash<ElaMetaTypeHash, QString> _allTypeHashMap;
    QHash<ElaMetaTypeHash, QString> _enumTypeHashMap;
    QHash<ElaMetaTypeHash, ElaMetaTypeInfo> _metaTypeMap;
    const ElaMetaTypeInfo& _getMetaTypeInfo(ElaMetaTypeHash metaTypeHash);
    QString _getMetaTypePath(ElaMetaTypeHash metaTypeHash);
    bool _isMetaTypeRegistered(ElaMetaTypeHash metaTypeHash) const;
    bool _isEnumTypeRegistered(ElaMetaTypeHash metaTypeHash) const;

    // pugixml
    void _serializeMetaType(const ElaMetaTypeInfo& metaTypeInfo, const ElaVariant& metaVariant, pugi::xml_node& metaNode);
    void _serializeElement(const ElaVariant& itemVariant, pugi::xml_node& elementNode);

    ElaVariant _deserializeMetaType(pugi::xml_node& metaNode);
    ElaVariant _deserializeElement(pugi::xml_node& elementNode);

    // QByteArray
    void _serializeMetaType(const ElaMetaTypeInfo& metaTypeInfo, const ElaVariant& metaVariant, ElaMetaBuffer& metaBuffer);
    void _serializeElement(const ElaVariant& itemVariant, ElaMetaBuffer& metaBuffer);

    ElaVariant _deserializeMetaType(ElaMetaBuffer& metaBuffer);
    ElaVariant _deserializeElement(ElaMetaBuffer& metaBuffer);
};

#endif //ELAMETATYPE_ELAMETATYPEPRIVATE_H
