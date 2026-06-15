#ifndef ELAMETATYPE_ELAMETATYPEMACRO_H
#define ELAMETATYPE_ELAMETATYPEMACRO_H

#include "ElaMetaCollection.h"
#include "ElaMultiFieldMacro.h"

namespace Ela
{
namespace MetaType
{
typedef int ElaMetaTypeId;
typedef quint32 ElaMetaTypeHash;
template <class MetaType>
struct ElaMetaDataRegister {
};

template <class MetaType>
inline Q_DECL_CONSTEXPR ElaMetaTypeHash getMetaTypeHash();

#define ELA_LINK_(A, B) A##B
#define ELA_LINK(A, B) ELA_LINK_(A, B)
#define ELA_CONCAT_(A, B) A.B
#define ELA_CONCAT(A, B) ELA_CONCAT_(A, B)
#define ELA_STRINGIFY_(A, B) #B
#define ELA_STRINGIFY(A, B) ELA_STRINGIFY_(A, B)

#define ELA_RECURSION_(MARCO, ...)                       \
    ELA_LINK(ELA_RECURSION_, ELA_ARG_COUNT(__VA_ARGS__)) \
    (MARCO, ##__VA_ARGS__)
#define ELA_RECURSION(MARCO, ...) ELA_RECURSION_(MARCO, ##__VA_ARGS__)

#define ELA_RECURSION_ARG_(MARCO, STRUCT, ...)               \
    ELA_LINK(ELA_RECURSION_ARG_, ELA_ARG_COUNT(__VA_ARGS__)) \
    (MARCO, STRUCT, ##__VA_ARGS__)
#define ELA_RECURSION_ARG(MARCO, STRUCT, ...) ELA_RECURSION_ARG_(MARCO, STRUCT, ##__VA_ARGS__)

#define ELA_FIELD(Field, FieldDesc) ElaMetaType::getInstance()->registerMetaField(metaTypeHash, ElaMetaField(metaTypeName, #Field, FieldDesc, ElaMetaMemberFieldOperatorPtr(&MetaTypeDef::Field)));
#define ELA_FIELDS_(Field) ELA_FIELD(Field, "Field")
#define ELA_FIELDS(...) \
    ELA_RECURSION(ELA_FIELDS_, ##__VA_ARGS__)

#define ELA_FUNC_FIELD(Field, GetFunc, SetFunc) ElaMetaType::getInstance()->registerMetaField(metaTypeHash, ElaMetaField(metaTypeName, Field, "FuncField", ElaMetaFuncFieldOperatorPtrHelper(&MetaTypeDef::GetFunc, &MetaTypeDef::SetFunc)));

#define ELA_META_FRIEND(MetaType) \
    friend class ElaMetaType;     \
    friend struct ElaMetaDataRegister<MetaType>;

#define ELA_FIELD_CAST(ParentMetaType)                                                                                                                                               \
    static_assert(std::is_base_of<ParentMetaType, MetaTypeDef>::value && !std::is_same<ParentMetaType, MetaTypeDef>::value, "This Type Is Not Derived Form " #ParentMetaType "..."); \
    ElaMetaType::getInstance()->registerParentMetaType(metaTypeHash, getMetaTypeHash<ParentMetaType>(), ElaMetaMemberFieldCasterPtr<MetaTypeDef, ParentMetaType>());

#define ELA_META_PARENT(...) \
    ELA_RECURSION(ELA_FIELD_CAST, ##__VA_ARGS__)

static Q_DECL_CONSTEXPR ElaMetaTypeHash generateHash(const char* str, quint32 value = 0x811C9DC5u) noexcept
{
    return *str ? generateHash(str + 1, (value ^ static_cast<ElaMetaTypeHash>(*str)) * 0x01000193u) : value;
}

#define ELA_META_HASH(MetaTypeName)                                                                       \
    template <>                                                                                           \
    inline Q_DECL_CONSTEXPR Ela::MetaType::ElaMetaTypeHash Ela::MetaType::getMetaTypeHash<MetaTypeName>() \
    {                                                                                                     \
        return generateHash(#MetaTypeName);                                                               \
    }

#define ELA_META_HASHES(...) \
    ELA_RECURSION(ELA_META_HASH, ##__VA_ARGS__)

#define ELA_DECLARE_BASE(MetaType) \
    Q_DECLARE_METATYPE(MetaType)

#define ELA_DECLARE_POINTER(MetaType) \
    Q_DECLARE_METATYPE(MetaType*)     \
    Q_DECLARE_METATYPE(std::shared_ptr<MetaType>)

#define ELA_META_CONTENT(...)                                                                                                                                                                    \
    auto constructorFunc = []() -> void* {                                                                                                                                                       \
        return new MetaTypeDef();                                                                                                                                                                \
    };                                                                                                                                                                                           \
    auto destructorFunc = [](void* obj) {                                                                                                                                                        \
        delete static_cast<MetaTypeDef*>(obj);                                                                                                                                                   \
    };                                                                                                                                                                                           \
    using StdSharedFuncPtr = std::shared_ptr<void> (*)(MetaTypeDef*);                                                                                                                            \
    using StdSharedFuncVoidPtr = std::shared_ptr<void> (*)(void*);                                                                                                                               \
    StdSharedFuncPtr constructorStdSharedFunc = ElaMetaTypeStdSharedPointer<MetaTypeDef>;                                                                                                        \
    ElaMetaTypeHash metaTypeHash = getMetaTypeHash<MetaTypeDef>();                                                                                                                               \
    ElaMetaType::getInstance()->registerMetaType(metaTypeName, metaTypeHash, metaTypeIdList, constructorFunc, destructorFunc, reinterpret_cast<StdSharedFuncVoidPtr>(constructorStdSharedFunc)); \
    ELA_META_PARENT(__VA_ARGS__)

#define ELA_BEGIN_QT_META(MetaType, ...)                                                                           \
    ELA_DECLARE_POINTER(MetaType)                                                                                  \
    ELA_META_HASH(MetaType)                                                                                        \
    template <>                                                                                                    \
    struct ElaMetaDataRegister<MetaType> {                                                                         \
        typedef MetaType MetaTypeDef;                                                                              \
        explicit ElaMetaDataRegister()                                                                             \
        {                                                                                                          \
            static_assert(std::is_base_of<QObject, MetaType>::value, #MetaType " Is Not Derived Form QObject..."); \
            QString metaTypeName = #MetaType;                                                                      \
            Q_UNUSED(metaTypeName);                                                                                \
            QList<ElaMetaTypeId> metaTypeIdList;                                                                   \
            metaTypeIdList.append(QMetaTypeId<MetaType*>::qt_metatype_id());                                       \
            metaTypeIdList.append(QMetaTypeId<MetaType*>::qt_metatype_id());                                       \
            metaTypeIdList.append(QMetaTypeId<std::shared_ptr<MetaType>>::qt_metatype_id());                       \
            ELA_META_CONTENT(__VA_ARGS__);

#define ELA_BEGIN_META(MetaType, ...)                                                        \
    ELA_DECLARE_BASE(MetaType)                                                               \
    ELA_DECLARE_POINTER(MetaType)                                                            \
    ELA_META_HASH(MetaType)                                                                  \
    template <>                                                                              \
    struct ElaMetaDataRegister<MetaType> {                                                   \
        typedef MetaType MetaTypeDef;                                                        \
        explicit ElaMetaDataRegister()                                                       \
        {                                                                                    \
            QString metaTypeName = #MetaType;                                                \
            Q_UNUSED(metaTypeName);                                                          \
            QList<ElaMetaTypeId> metaTypeIdList;                                             \
            metaTypeIdList.append(QMetaTypeId<MetaType>::qt_metatype_id());                  \
            metaTypeIdList.append(QMetaTypeId<MetaType*>::qt_metatype_id());                 \
            metaTypeIdList.append(QMetaTypeId<std::shared_ptr<MetaType>>::qt_metatype_id()); \
            ELA_META_CONTENT(__VA_ARGS__);

#define ELA_END_META()                                                        \
    }                                                                         \
    static void instantiation()                                               \
    {                                                                         \
        ElaMetaCollection<ElaMetaDataRegister<MetaTypeDef>>::instantiation(); \
    }                                                                         \
    }                                                                         \
    ;

#define ELA_ENUM_META(EnumType)                                                                   \
    ELA_DECLARE_BASE(EnumType)                                                                    \
    ELA_META_HASH(EnumType)                                                                       \
    template <>                                                                                   \
    struct ElaMetaDataRegister<EnumType> {                                                        \
        typedef EnumType EnumTypeDef;                                                             \
        explicit ElaMetaDataRegister()                                                            \
        {                                                                                         \
            ElaMetaType::getInstance()->registerMetaEnum(#EnumType, getMetaTypeHash<EnumType>()); \
        }                                                                                         \
        static void instantiation()                                                               \
        {                                                                                         \
            ElaMetaCollection<ElaMetaDataRegister<EnumTypeDef>>::instantiation();                 \
        }                                                                                         \
    };
} // namespace MetaType
} // namespace Ela
using namespace Ela::MetaType;
#endif //ELAMETATYPE_ELAMETATYPEMACRO_H
