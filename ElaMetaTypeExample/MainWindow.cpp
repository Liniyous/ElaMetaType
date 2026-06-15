#include "MainWindow.h"

#include "ElaMetaType.h"
#include "TestDefine.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // auto testMainData = ElaMetaType::getInstance()->readFromFile("./TestByte.data", true).value<MainData*>();
    // if (testMainData)
    // {
    //     qDebug() << testMainData->data1->member4;
    // }

    auto mainData1 = new MainData();
    mainData1->data0.baseEnum = Invalid;
    mainData1->data1 = std::make_shared<ChildData>();
    mainData1->data1->base2 = "asdiaklsd";
    mainData1->data1->member2 = "Child1";

    mainData1->data2 = "Test1111";
    mainData1->data3 = 112;
    ChildData childData;
    childData.member1 = "ceshizifu";
    childData.member3 = 115;
    childData.member4 = QPoint(1, 2);
    mainData1->data5.append(childData);

    mainData1->data6 = "测试原生字符";
    mainData1->data7.append(1);
    mainData1->data7.append(2);
    mainData1->data7.append(3);
    mainData1->data8.append("11");
    mainData1->data8.append("22");
    mainData1->data8.append("33");
    mainData1->data9.emplace_back("44");
    mainData1->data9.emplace_back("55");
    mainData1->data9.emplace_back("66");

    mainData1->data10.append(1.11);
    mainData1->data10.append(2.22);
    mainData1->data10.append(3.33);
    mainData1->data11.insert(3, childData);
    mainData1->data11.insert(4, childData);
    mainData1->data11.insert(5, ChildData());
    mainData1->data12.insert(std::make_pair(4, childData));
    mainData1->data12.insert(std::make_pair(5, childData));
    mainData1->data12.insert(std::make_pair(6, ChildData()));

    mainData1->data13.append(ElaVariant::fromValue(1));
    mainData1->data13.append(ElaVariant::fromValue(QString("33")));
    mainData1->data13.append(ElaVariant::fromValue(childData));
    mainData1->data13.append(ElaVariant::fromValue(mainData1->data5));
    mainData1->data13.append(ElaVariant::fromValue(mainData1->data12));

    mainData1->data14 = QPair<QString, ChildData>("112233", childData);
    auto testBaseData = new BaseData1();
    testBaseData->base1 = 12;
    testBaseData->base2 = "Base2";
    testBaseData->base0 = "Base0";
    mainData1->data15.push_back(testBaseData);
    mainData1->data15.push_back(testBaseData);

    mainData1->data16 = "函数字段";

    auto data17 = new ChildData();
    data17->member1 = "123123123123";
    mainData1->data17 = ElaVariant::fromValue(data17);
    mainData1->data18.append(1);
    mainData1->data18.append(2);
    mainData1->data18.append(3);

    mainData1->data19.insert("条目1", mainData1->data5);
    mainData1->data19.insert("条目2", QList<ChildData>());

    mainData1->data20.push_back(mainData1->data19);
    mainData1->data20.emplace_back();
    mainData1->data20.push_back(mainData1->data19);
    mainData1->data21 = "序列化测试A";
    mainData1->data22 = std::make_tuple<QString, int, ChildData>("String", 100, ChildData());
    mainData1->data23 = 12347654;
    mainData1->setWindowTitle("序列化窗口");
    for (int i = 0; i < 20000; i++)
    {
        mainData1->data5.append(childData);
    }
    //mainData1->show();

    // 序列化
    QElapsedTimer timer;
    timer.start();
    auto serializeStr = ElaMetaType::getInstance()->serialize(mainData1);
    qDebug() << "Serialize Time:" << timer.elapsed() << "MS" << "Size:" << serializeStr.count() / 1024.0 / 1024.0 << "MB";
    // timer.start();
    ElaMetaType::saveToFile("./Test.xml", serializeStr);

    // qDebug() << "Save Time:" << timer.elapsed();

    // 反序列化
    timer.start();
    auto deMainData = ElaMetaType::getInstance()->deserialize<MainData*>(serializeStr);
    qDebug() << "Deserialize Time:" << timer.elapsed() << "MS";
    deMainData->setWindowTitle("反序列化窗口");
    //deMainData->show();
    serializeStr = ElaMetaType::getInstance()->serialize(ElaVariant::fromValue(deMainData));
    ElaMetaType::saveToFile("./TestDe.xml", serializeStr);
    QApplication::processEvents();
    int index = 0;
    while (index++ < 10)
    {
        // 二进制序列化
        timer.start();
        auto metaByte = ElaMetaType::getInstance()->serializeToByte(mainData1);
        qDebug() << "SerializeToByte Time:" << timer.nsecsElapsed() / 1000 << "us" << "Size:" << metaByte.size() / 1024.0 / 1024.0 << "MB";
        // ElaMetaType::saveToFile("./TestByte.data", metaByte);

        // 二进制反序列化
        timer.start();
        auto deByteMainData = ElaMetaType::getInstance()->deserializeFromByte<MainData*>(metaByte);
        // qDebug() << deByteMainData->data17.value<ChildData*>()->member1;
        qDebug() << "DeserializeFromByte Time:" << timer.nsecsElapsed() / 1000 << "us" << deByteMainData;
        // ElaMetaType::getInstance()->saveToFile("./TestByte.xml", ElaVariant::fromValue(deByteMainData));
        delete deByteMainData;
    }
}

MainWindow::~MainWindow()
{
}
