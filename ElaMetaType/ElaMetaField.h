#ifndef ELAMETATYPE_ELAMETAFIELD_H
#define ELAMETATYPE_ELAMETAFIELD_H

#include "ElaVariant.h"

class ElaMetaFieldOperator
{
public:
    virtual ~ElaMetaFieldOperator() = default;
    virtual void setValue(void* obj, const ElaVariant& Val) = 0;
    virtual ElaVariant getValue(const void* obj) = 0;
};

template <typename MetaType, typename ValueType, typename FieldType>
class ElaMetaMemberFieldOperator : public ElaMetaFieldOperator
{
public:
    explicit ElaMetaMemberFieldOperator(FieldType field)
        : _field(field)
    {
    }

    void setValue(void* obj, const ElaVariant& val) override
    {
        static_cast<MetaType*>(obj)->*_field = val.value<ValueType>();
    }

    ElaVariant getValue(const void* obj) override
    {
        return ElaVariant::fromValue<ValueType>(static_cast<const MetaType*>(obj)->*_field);
    }

private:
    FieldType _field;
};

template <typename MetaType, typename ValueType, typename GetFuncType, typename SetFuncType>
class ElaMetaFuncFieldOperator : public ElaMetaFieldOperator
{
public:
    explicit ElaMetaFuncFieldOperator(GetFuncType getFunc, SetFuncType setFunc)
        : _getFunc(getFunc), _setFunc(setFunc)
    {
    }

    void setValue(void* obj, const ElaVariant& val) override
    {
        (static_cast<MetaType*>(obj)->*_setFunc)(val.value<ValueType>());
    }

    ElaVariant getValue(const void* obj) override
    {
        return ElaVariant::fromValue<ValueType>((static_cast<const MetaType*>(obj)->*_getFunc)());
    }

private:
    GetFuncType _getFunc;
    SetFuncType _setFunc;
};

class ElaMetaFieldCaster
{
public:
    virtual ~ElaMetaFieldCaster() = default;
    virtual const void* cast(const void* obj) = 0;
    virtual void* cast(void* obj) = 0;
};

template <typename MetaType, typename ParentMetaType>
class ElaMetaMemberFieldCaster : public ElaMetaFieldCaster
{
public:
    explicit ElaMetaMemberFieldCaster() = default;
    const void* cast(const void* obj) override
    {
        // 传入的为子类指针
        const auto metaTypePointer = static_cast<const MetaType*>(obj);
        // 自动转换内存偏移
        const ParentMetaType* parentMetaTypePointer = metaTypePointer;
        return parentMetaTypePointer;
    }
    void* cast(void* obj) override
    {
        auto metaTypePointer = static_cast<MetaType*>(obj);
        ParentMetaType* parentMetaTypePointer = metaTypePointer;
        return parentMetaTypePointer;
    }
};

template <typename MetaType, typename ValueType>
std::shared_ptr<ElaMetaFieldOperator> ElaMetaMemberFieldOperatorPtr(ValueType MetaType::* field)
{
    return std::make_shared<ElaMetaMemberFieldOperator<MetaType, ValueType, ValueType MetaType::*>>(field);
}

template <typename MetaType, typename ValueType, typename GetFuncType, typename SetFuncType>
std::shared_ptr<ElaMetaFieldOperator> ElaMetaFuncFieldOperatorPtr(GetFuncType getFunc, SetFuncType setFunc)
{
    return std::make_shared<ElaMetaFuncFieldOperator<MetaType, ValueType, GetFuncType, SetFuncType>>(getFunc, setFunc);
}

template <typename MetaType, typename ValueType, typename ReturnType>
std::shared_ptr<ElaMetaFieldOperator> ElaMetaFuncFieldOperatorPtrHelper(ReturnType (MetaType::*getFunc)() const, void (MetaType::*setFunc)(ValueType))
{
    typedef typename std::remove_reference<ValueType>::type NoRefValueType;
    typedef typename std::remove_const<NoRefValueType>::type NoConstNoRefValueType;

    typedef typename std::remove_reference<ReturnType>::type NoRefReturnType;
    typedef typename std::remove_const<NoRefReturnType>::type NoConstNoRefReturnType;
    static_assert(std::is_same<NoConstNoRefValueType, NoConstNoRefReturnType>::value, "ElaMetaType: The GetFunc and SetFunc Have Inconsistent Parameters");
    return ElaMetaFuncFieldOperatorPtr<MetaType, NoConstNoRefValueType>(getFunc, setFunc);
}

template <typename MetaType, typename ParentMetaType>
std::shared_ptr<ElaMetaFieldCaster> ElaMetaMemberFieldCasterPtr()
{
    return std::make_shared<ElaMetaMemberFieldCaster<MetaType, ParentMetaType>>();
}

struct ElaMetaField {
    explicit ElaMetaField() = default;
    explicit ElaMetaField(QString className, QString fieldName, QString fieldDesc, std::shared_ptr<ElaMetaFieldOperator> pOperator)
        : _className(std::move(className)), _fieldName(std::move(fieldName)), _fieldDesc(std::move(fieldDesc)), _operator(std::move(pOperator))
    {
    }
    bool operator==(const ElaMetaField& other) const
    {
        if (other._fieldName == _fieldName)
        {
            return true;
        }
        return false;
    }
    QString _className;
    QString _fieldName;
    QString _fieldDesc;
    std::shared_ptr<ElaMetaFieldOperator> _operator;
    QVector<std::shared_ptr<ElaMetaFieldCaster>> _casterList;
};

template <typename T>
std::shared_ptr<void> ElaMetaTypeStdSharedPointer(T* value)
{
    if (value)
    {
        return std::shared_ptr<void>(value);
    }
    return std::make_shared<T>();
}

#endif //ELAMETATYPE_ELAMETAFIELD_H
