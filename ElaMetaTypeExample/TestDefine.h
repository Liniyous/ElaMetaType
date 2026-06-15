#ifndef TEST__TESTDEFINE_H_
#define TEST__TESTDEFINE_H_

#include "ElaMetaType.h"
#include <QObject>
#include <QQueue>
#include <QString>

enum TestEnum
{
    Invalid = 0x0000,
    Valid = 0x0010
};
ELA_ENUM_META(TestEnum)

struct BaseData0 {
    QString base0{"000"};
    TestEnum baseEnum{Valid};
};
ELA_BEGIN_META(BaseData0)
ELA_FIELD(base0, "基础0")
ELA_FIELD(baseEnum, "基础枚举")
ELA_END_META()

struct BaseData1 : BaseData0 {
    long long base1{1234567890123};
    QString base2{"222"};
};
ELA_BEGIN_META(BaseData1, BaseData0)
ELA_FIELD(base1, "基础1")
ELA_FIELD(base2, "基础2")
ELA_END_META()

struct BaseData2 {
    QStringList base3{"1", "2", "3"};
    QString base4{"333"};
};
ELA_BEGIN_META(BaseData2)
ELA_FIELD(base3, "基础3")
ELA_FIELD(base4, "基础4")
ELA_END_META()

//ELA_FIELDS 生成的字段描述默认为Field
struct ChildData : BaseData1, BaseData2 {
    QString member1{"测试字符1"};
    QString member2{"测试字符2"};
    int member3{0};
    QPoint member4{12, 240};
};
ELA_BEGIN_META(ChildData, BaseData1, BaseData2)
ELA_FIELDS(member1, member2, member3, member4)
ELA_END_META()

class MainData : public QWidget, public ChildData
{
    Q_OBJECT
    // 友元声明 帮助class权限控制 Struct不需要
    ELA_META_FRIEND(MainData)
public:
    explicit MainData(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setWindowTitle("MainData");
        resize(600, 600);
    }
    ~MainData() override = default;

    void setData16(const QString& data)
    {
        data16 = data;
    }
    QString getData16() const
    {
        return data16;
    }

private:
    friend class MainWindow;
    BaseData1 data0;
    std::shared_ptr<ChildData> data1;
    QString data2{"TestMain"};
    int data3{3};
    QStringList data4{"1", "2", "3"};
    QList<ChildData> data5;
    std::string data6{"1212"};
    QList<int> data7;
    QList<std::string> data8;
    std::vector<std::string> data9;
    QVector<qreal> data10;
    QMap<int, ChildData> data11;
    std::map<int, ChildData> data12;
    ElaVariantList data13;
    QPair<QString, ChildData> data14;
    std::list<BaseData1*> data15;
    QString data16{"12321"};
    ElaVariant data17;
    QQueue<int> data18;
    QMap<QString, QList<ChildData>> data19;
    std::vector<QMap<QString, QList<ChildData>>> data20;
    QByteArray data21;
    std::tuple<QString, int, ChildData> data22;
    long data23;
};
ELA_BEGIN_QT_META(MainData, ChildData)
ELA_FIELD(data0, "继承测试")
ELA_FIELD(data1, "ChildData类型测试")
ELA_FIELD(data2, "QString类型测试")
ELA_FIELD(data3, "int类型测试")
ELA_FIELD(data4, "QStringList类型测试")
ELA_FIELD(data5, "QList_ChildData_类型测试")
ELA_FIELD(data6, "string类型测试")
ELA_FIELD(data7, "QList_int_类型测试")
ELA_FIELD(data8, "QList_string_类型测试")
ELA_FIELD(data9, "vector_string_类型测试")
ELA_FIELD(data10, "QVector_qreal_类型测试")
ELA_FIELD(data11, "QMap类型测试")
ELA_FIELD(data12, "StdMap类型测试")
ELA_FIELD(data13, "ElaVariantList类型测试")
ELA_FIELD(data14, "QPair类型测试")
ELA_FIELD(data15, "指针List类型测试")
ELA_FUNC_FIELD("测试函数字段", getData16, setData16)
ELA_FIELD(data17, "ElaVariant类型测试")
ELA_FIELD(data18, "QQueue类型测试")
ELA_FIELD(data19, "多重容器1测试")
ELA_FIELD(data20, "多重容器2测试")
ELA_FIELD(data21, "QByteArray测试")
ELA_FIELD(data22, "StdTuple测试")
ELA_FIELD(data23, "long测试")
ELA_END_META()

#endif //TEST__TESTDEFINE_H_
