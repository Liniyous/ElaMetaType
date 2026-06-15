#ifndef ELAMETATYPE_ELAMETACOLLECTION_H_
#define ELAMETATYPE_ELAMETACOLLECTION_H_

#include "ElaMetaTypeExport.h"

template <typename MetaType>
class ElaMetaCollection
{
public:
    Q_DECL_EXPORT static ElaMetaCollection& instantiation()
    {
        static ElaMetaCollection metaCollection;
        (void)_instance;
        static MetaType metaValue;
        return metaCollection;
    }

private:
    Q_DECL_EXPORT static ElaMetaCollection& _instance;
};

template <typename MetaType>
Q_DECL_EXPORT ElaMetaCollection<MetaType>& ElaMetaCollection<MetaType>::_instance = ElaMetaCollection::instantiation();

#endif //ELAMETATYPE_ELAMETACOLLECTION_H_
