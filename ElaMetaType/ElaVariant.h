#ifndef ELAMETATYPE_ELAVARIANT_H_
#define ELAMETATYPE_ELAVARIANT_H_
#include "ElaMetaTypeMacro.h"
#include "ElaMetaTypeExport.h"
#include <QDebug>
#include <QList>
#include <QObject>
#include <QVariant>
#include <deque>
#include <memory>
#include <set>
template <typename T>
struct ElaVariantWrapper;

class ElaVariant;
typedef QPair<ElaVariant, ElaVariant> ElaVariantPair;
typedef QList<ElaVariant> ElaVariantList;
typedef QVector<ElaVariant> ElaVariantVector;

ELA_META_HASHES(bool, int, uint, qlonglong, qulonglong, double,
                short, char, long, ulong, ushort, uchar,
                float, signed char)
ELA_META_HASHES(QChar, QString, QStringList, QDate, QTime, QDateTime,
                QRect, QRectF, QSize, QSizeF, QLine, QLineF,
                QPoint, QPointF, QColor, QPixmap, QImage, QByteArray)
ELA_META_HASHES(std::string, ElaVariantPair, ElaVariantList)

class ELA_METATYPE_EXPORT ElaVariant : public QVariant
{
public:
    enum MetaFlag
    {
        Stack = 0,
        Heap,
        StdSharedPointer,
    };
    ElaVariant();
    ElaVariant(const ElaVariant& other);
    ElaVariant(ElaMetaTypeHash metaHash, int typeId, const void* copy, ElaVariant::MetaFlag flags);
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    explicit ElaVariant(const std::string& string);
    explicit ElaVariant(const QStringList& qStringList);
#endif

    template <typename T>
    static ElaVariant fromValue(const T& value)
    {
        return ElaVariantWrapper<std::remove_const_t<T>>::fromValue(value);
    }

    template <typename T>
    Q_REQUIRED_RESULT T value() const
    {
        return ElaVariantWrapper<std::remove_const_t<T>>::value(*this);
    }

    Q_REQUIRED_RESULT QString getTypeName() const;
    Q_REQUIRED_RESULT ElaVariant::MetaFlag getMetaFlag() const;
    Q_REQUIRED_RESULT ElaMetaTypeHash getMetaHash() const;

private:
    QString _typeName{""};
    ElaVariant::MetaFlag _metaFlag{MetaFlag::Stack};
    ElaMetaTypeHash _metaHash{0};
};
Q_DECLARE_METATYPE(ElaVariant)
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE(std::set)
Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE(std::deque)

template <typename T>
struct ElaVariantWrapper {
    static ElaVariant fromValue(const T& value)
    {
        return ElaVariant(getMetaTypeHash<T>(), qMetaTypeId<T>(), &value, ElaVariant::MetaFlag::Stack);
    }
    static T value(const ElaVariant& variant)
    {
        return variant.QVariant::value<T>();
    }
};

template <typename T>
struct ElaVariantWrapper<T*> {
    static ElaVariant fromValue(const T* value)
    {
        if (value)
        {
            return ElaVariant(getMetaTypeHash<T>(), QMetaTypeId<T*>::qt_metatype_id(), &value, ElaVariant::MetaFlag::Heap);
        }
        return {};
    }
    static T* value(const ElaVariant& variant)
    {
        return variant.QVariant::value<T*>();
    }
};

template <typename T>
struct ElaVariantWrapper<std::shared_ptr<T>> {
    typedef std::shared_ptr<T> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        if (value)
        {
            return ElaVariant(getMetaTypeHash<T>(), QMetaTypeId<TYPE>::qt_metatype_id(), &value, ElaVariant::MetaFlag::StdSharedPointer);
        }
        return {};
    }
    static TYPE value(const ElaVariant& variant)
    {
        return variant.QVariant::value<TYPE>();
    }
};

template <>
struct ElaVariantWrapper<ElaVariant> {
    static ElaVariant fromValue(const ElaVariant& value)
    {
        return value;
    }
    static ElaVariant value(const ElaVariant& variant)
    {
        return variant;
    }
};

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
template <>
struct ElaVariantWrapper<std::string> {
    static ElaVariant fromValue(const std::string& value)
    {
        return ElaVariant(value);
    }
    static std::string value(const ElaVariant& variant)
    {
        return variant.QVariant::value<std::string>();
    }
};

template <>
struct ElaVariantWrapper<QStringList> {
    static ElaVariant fromValue(const QStringList& value)
    {
        return ElaVariant(value);
    }
    static QStringList value(const ElaVariant& variant)
    {
        return variant.QVariant::value<QStringList>();
    }
};
#endif

template <typename T>
struct ElaVariantWrapper<std::set<T>> {
    typedef std::set<T> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        for (const auto& var: value)
        {
            ElaVariant variant = ElaVariant::fromValue(var);
            varList.append(variant);
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE set;
        for (const auto& var: varList)
        {
            set.insert(var.value<T>());
        }
        return set;
    }
};

template <typename T>
struct ElaVariantWrapper<std::list<T>> {
    typedef std::list<T> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        for (const auto& var: value)
        {
            ElaVariant variant = ElaVariant::fromValue(var);
            varList.append(variant);
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE list;
        for (const auto& var: varList)
        {
            list.push_back(var.value<T>());
        }
        return list;
    }
};

template <typename T, typename Alloc>
struct ElaVariantWrapper<std::vector<T, Alloc>> {
    typedef std::vector<T, Alloc> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        for (const auto& var: value)
        {
            ElaVariant variant = ElaVariant::fromValue(var);
            varList.append(variant);
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE vector;
        for (const auto& var: varList)
        {
            vector.push_back(var.value<T>());
        }
        return vector;
    }
};

template <typename T1, typename T2>
struct ElaVariantWrapper<std::pair<T1, T2>> {
    typedef std::pair<T1, T2> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantPair pair = ElaVariantPair(ElaVariant::fromValue(value.first), ElaVariant::fromValue(value.second));
        return {getMetaTypeHash<ElaVariantPair>(), QMetaTypeId<ElaVariantPair>::qt_metatype_id(), &pair, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto pair = variant.QVariant::value<QPair<ElaVariant, ElaVariant>>();
        if (!pair.first.isValid())
        {
            return variant.QVariant::value<TYPE>();
        }
        return TYPE(pair.first.value<T1>(), pair.second.value<T2>());
    }
};

template <typename Key, typename Value>
struct ElaVariantWrapper<std::map<Key, Value>> {
    typedef std::map<Key, Value> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        for (typename TYPE::const_iterator it = value.begin(); it != value.end(); it++)
        {
            varList.append(ElaVariant::fromValue(QPair<Key, Value>(it->first, it->second)));
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE map;
        for (const auto& var: varList)
        {
            auto pair = var.value<QPair<ElaVariant, ElaVariant>>();
            map[pair.first.value<Key>()] = pair.second.value<Value>();
        }
        return map;
    }
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template <typename T1, typename T2>
struct ElaVariantWrapper<QPair<T1, T2>> {
    typedef QPair<T1, T2> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantPair pair = ElaVariantPair(ElaVariant::fromValue(value.first), ElaVariant::fromValue(value.second));
        return {getMetaTypeHash<ElaVariantPair>(), QMetaTypeId<ElaVariantPair>::qt_metatype_id(), &pair, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto pair = variant.QVariant::value<QPair<ElaVariant, ElaVariant>>();
        if (!pair.first.isValid())
        {
            return variant.QVariant::value<TYPE>();
        }
        return TYPE(pair.first.value<T1>(), pair.second.value<T2>());
    }
};
#endif

template <typename T>
struct ElaVariantWrapper<QSet<T>> {
    typedef QSet<T> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        for (const auto& var: value)
        {
            ElaVariant variant = ElaVariant::fromValue(var);
            varList.append(variant);
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE set;
        for (const auto& var: varList)
        {
            set.insert(var.value<T>());
        }
        return set;
    }
};

template <typename T>
struct ElaVariantWrapper<QList<T>> {
    typedef QList<T> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        varList.reserve(value.count());
        for (const auto& var: value)
        {
            ElaVariant variant = ElaVariant::fromValue(var);
            varList.append(variant);
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE list;
        list.reserve(varList.count());
        for (const auto& var: varList)
        {
            list.append(var.value<T>());
        }
        return list;
    }
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template <typename T>
struct ElaVariantWrapper<QVector<T>> {
    typedef QVector<T> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        varList.reserve(value.count());
        for (const auto& var: value)
        {
            ElaVariant variant = ElaVariant::fromValue(var);
            varList.append(variant);
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE vector;
        vector.reserve(varList.count());
        for (const auto& var: varList)
        {
            vector.append(var.value<T>());
        }
        return vector;
    }
};
#endif

template <typename T>
struct ElaVariantWrapper<QQueue<T>> {
    typedef QQueue<T> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        for (const auto& var: value)
        {
            ElaVariant variant = ElaVariant::fromValue(var);
            varList.append(variant);
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE queue;
        for (const auto& var: varList)
        {
            queue.append(var.value<T>());
        }
        return queue;
    }
};

template <typename Key, typename Value>
struct ElaVariantWrapper<QMap<Key, Value>> {
    typedef QMap<Key, Value> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        for (typename TYPE::const_iterator it = value.begin(); it != value.end(); ++it)
        {
            varList.append(ElaVariant::fromValue(QPair<Key, Value>(it.key(), it.value())));
        }
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        auto varList = variant.QVariant::value<ElaVariantList>();
        if (varList.isEmpty())
        {
            return variant.QVariant::value<TYPE>();
        }
        TYPE map;
        for (const auto& var: varList)
        {
            auto pair = var.value<QPair<ElaVariant, ElaVariant>>();
            map[pair.first.value<Key>()] = pair.second.value<Value>();
        }
        return map;
    }
};

template <size_t Index, typename... T>
struct ElaTupleWrapper {
    static void tupleToArray(const std::tuple<T...>& tuple, ElaVariantList& varList)
    {
        ElaTupleWrapper<Index - 1, T...>::tupleToArray(tuple, varList);
        varList.append(ElaVariant::fromValue(std::get<Index - 1>(tuple)));
    }

    static void arrayToTuple(ElaVariantList& varList, std::tuple<T...>& tuple)
    {
        using TupleElementType = std::tuple_element_t<Index - 1, std::tuple<T...>>;
        std::get<Index - 1>(tuple) = varList[Index - 1].value<TupleElementType>();
        ElaTupleWrapper<Index - 1, T...>::arrayToTuple(varList, tuple);
    }
};

template <typename... T>
struct ElaTupleWrapper<0, T...> {
    static void tupleToArray(const std::tuple<T...>& tuple, ElaVariantList& varList) {}
    static void arrayToTuple(ElaVariantList& varList, std::tuple<T...>& tuple) {}
};

template <typename... T>
struct ElaVariantWrapper<std::tuple<T...>> {
    typedef std::tuple<T...> TYPE;
    static ElaVariant fromValue(const TYPE& value)
    {
        ElaVariantList varList;
        Q_DECL_CONSTEXPR int count = sizeof...(T);
        ElaTupleWrapper<count, T...>::tupleToArray(value, varList);
        return {getMetaTypeHash<ElaVariantList>(), QMetaTypeId<ElaVariantList>::qt_metatype_id(), &varList, ElaVariant::MetaFlag::Stack};
    }

    static TYPE value(const ElaVariant& variant)
    {
        Q_DECL_CONSTEXPR int count = sizeof...(T);
        auto varList = variant.QVariant::value<ElaVariantList>();
        TYPE tuple;
        if (varList.isEmpty())
        {
            return tuple;
        }
        ElaTupleWrapper<count, T...>::arrayToTuple(varList, tuple);
        return tuple;
    }
};

#endif //ELAMETATYPE_ELAVARIANT_H_
