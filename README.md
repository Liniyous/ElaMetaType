# ElaMetaType

本库是一个基于 Qt 元对象系统的通用 C++/QT 序列化/反序列化框架。通过宏驱动的编译期类型注册机制，将任意结构体或 QObject
派生类实例序列化为可读的
XML 字符串或高效的二进制字节流，并支持完整还原。

## 重要提示

当前版本为 ElaMetaType 开源试用版，实现机制与闭源正式版相差较大，各项性能指标约为闭源正式版的十分之一，且可能存在部分已知或未知缺陷。我们
**不建议**在性能与稳定性要求严苛的生产环境中直接使用此版本。

以下为同一测试样本（20000+ 条嵌套对象）在各版本下的基准对比，测试CPU为i9-14900HX：

| 版本              | 序列化耗时 | 反序列化耗时 | 数据体积    | 
|-----------------|-------|--------|---------|
| ElaMetaType 开源版 | 42 ms | 72 ms  | 2.75 MB | 
| ElaMetaType 闭源版 | 4 ms  | 7 ms   | 1.22 MB |
| ProtoBuf        | 5 ms  | 10 ms  | 1.36 MB |

> 闭源版二进制序列化及反序列化速度约为 ProtoBuf 的 **1.3 倍**，体积减少约 **10%**，使用方式与开源版完全一致。可联系作者获取完整基准对比报告。

如需获取闭源正式版及商业授权支持，请加入 **ElaWidgetTools 交流群 850243692** 联系作者购买。

---

## 目录

- [1. 设计目标](#1-设计目标)
- [2. 核心架构](#2-核心架构)
- [3. 功能特性](#3-功能特性)
- [4. 依赖](#4-依赖)
- [5. 快速开始](#5-快速开始)
- [6. 定义可序列化类型](#6-定义可序列化类型)
- [7. 继承](#7-继承)
- [8. 函数字段](#8-函数字段)
- [9. 序列化与反序列化](#9-序列化与反序列化)
- [10. 文件读写](#10-文件读写)
- [11. 支持的容器类型](#11-支持的容器类型)
- [12. 指针与智能指针](#12-指针与智能指针)
- [13. 类型检查](#13-类型检查)
- [14. 注意事项](#14-注意事项)

---

## 1. 设计目标

- **零侵入**：通过宏声明字段，无需修改原有数据结构逻辑
- **高性能**：二进制通道支持 LZ4 压缩，适合大量数据的快速传输与存储
- **全类型覆盖**：支持基础类型、Qt 类型、STL 容器、嵌套自定义类型、指针、智能指针等
- **继承友好**：正确处理多重继承（含菱形继承）中字段的遍历与去重

---

## 2. 核心架构

```
┌─────────────────────────────────────────────┐
│               ElaMetaType (单例)             │
│  serialize() / deserialize()       ← XML    │
│  serializeToByte() / deserializeFromByte()   │
│  saveToFile() / readFromFile()               │
└──────────────────┬──────────────────────────┘
                   │ 调度
     ┌─────────────┼─────────────┐
     ▼             ▼             ▼
┌─────────┐ ┌──────────┐ ┌──────────────┐
│ pugixml │ │ElaMetaBuffer│ │ LZ4 压缩     │
│(XML引擎)│ │(二进制缓冲) │ │ (可选压缩)   │
└─────────┘ └──────────┘ └──────────────┘
     ▲                           ▲
     └───────────┬───────────────┘
                 │
     ┌───────────┴───────────┐
     │   ElaMetaTypePrivate  │
     │  (类型注册表 & 引擎)   │
     └───────────────────────┘
                 │
     ┌───────────┴───────────┐
     │   ElaMetaTypeInfo     │
     │  (每种注册类型的元信息) │
     │  - 类型名 / 哈希 / 字段列表 │
     │  - 构造函数 / 析构函数     │
     │  - 父类型列表 & 字段转换器  │
     └───────────────────────┘
```

| 组件                     | 职责                                |
|------------------------|-----------------------------------|
| `ElaMetaType`          | 对外 API 的统一入口，单例模式                 |
| `ElaMetaTypePrivate`   | 内部引擎，持有所有注册类型的元信息表，负责序列化/反序列化调度   |
| `ElaMetaTypeInfo`      | 描述一个注册类型的完整元信息                    |
| `ElaVariant`           | 扩展 `QVariant`，额外记录类型哈希、类型名、内存管理标志 |
| `ElaMetaField`         | 描述一个字段：名称、描述、读写访问器、父类转换器链         |
| `ElaMetaFieldOperator` | 抽象字段访问器，通过模板特化实现不同类型字段的 get/set   |
| `ElaMetaFieldCaster`   | 多继承场景下，将子类指针偏移到对应基类的转换器           |
| `ElaMetaBuffer`        | 自定义二进制缓冲区，支持大小端转换、动态扩容            |

---

## 3. 功能特性

### 3.1 双通道序列化

| 通道     | 方法                                            | 输出格式         | 特点                   |
|--------|-----------------------------------------------|--------------|----------------------|
| XML 文本 | `serialize()` / `deserialize()`               | XML 字符串      | 人类可读，适合调试、配置文件       |
| 二进制    | `serializeToByte()` / `deserializeFromByte()` | `QByteArray` | 速度极快，体积小，适合网络传输、大量数据 |

二进制通道内置 **LZ4 压缩** 选项，以极快的压缩/解压速度在体积和延迟之间取得平衡。

### 3.2 字段注册方式

| 宏                                             | 用途                         |
|-----------------------------------------------|----------------------------|
| `ELA_FIELD(Field, FieldDesc)`                 | 注册单个成员字段，附带中文描述            |
| `ELA_FIELDS(...)`                             | 批量注册多个字段，描述默认为 "Field"     |
| `ELA_FUNC_FIELD(FieldDesc, GetFunc, SetFunc)` | 注册函数字段，通过 getter/setter 访问 |

### 3.3 支持的类型

**基础类型**：`bool`, `int`, `uint`, `short`, `long`, `ulong`, `float`, `double`, `char`, `signed char`, `uchar`,
`qlonglong`, `qulonglong`

**Qt 类型**：`QChar`, `QString`, `QStringList`, `QDate`, `QTime`, `QDateTime`, `QRect`, `QRectF`, `QSize`, `QSizeF`,
`QLine`, `QLineF`, `QPoint`, `QPointF`, `QColor`, `QPixmap`, `QImage`, `QByteArray`

**STL 类型**：`std::string`, `std::pair<K,V>`, `std::tuple<T...>`, `std::vector<T>`, `std::list<T>`, `std::set<T>`,
`std::map<K,V>`

**Qt 容器**：`QList<T>`, `QVector<T>`, `QMap<K,V>`, `QSet<T>`, `QQueue<T>`, `QPair<K,V>`（Qt5）

以上所有类型支持任意深度嵌套，以及 `ElaVariantList`（异构容器）、`ElaVariantPair`。

### 3.4 内存管理标志

| 标志                 | 含义                   | 场景                 |
|--------------------|----------------------|--------------------|
| `Stack`            | 栈对象 / 值类型            | 普通值传递              |
| `Heap`             | 堆指针 (`T*`)           | 裸指针，序列化时自动 new 并拷贝 |
| `StdSharedPointer` | `std::shared_ptr<T>` | 智能指针管理，共享所有权       |

### 3.5 继承支持

- **多重继承**：自动向上收集所有基类的注册字段
- **菱形继承**：同一字段在继承链中只注册一次，自动去重
- **Caster 链**：每个继承层级保存指针偏移转换器，反序列化时依次执行

### 3.6 文件 I/O

`saveToFile()` / `readFromFile()` 内部使用 `QFile::map()` 内存映射，高效处理大文件。

### 3.7 枚举支持

`ELA_ENUM_META(EnumType)` 将枚举注册到元类型系统，以枚举名字符串形式存取，保证版本兼容性。

### 3.8 类型哈希

每种注册类型在编译期通过 FNV-1a 算法基于类型名字符串生成唯一的 `ElaMetaTypeHash`（`quint32`），用于运行时快速查找类型注册信息。

---

## 4. 依赖

| 依赖                | 用途                |
|-------------------|-------------------|
| Qt 5.9+ (Widgets) | 基础框架、元对象系统、GUI 类型 |
| pugixml           | XML 解析与生成         |
| LZ4 1.10.0        | 二进制数据压缩           |
| C++11及以上          | 编译标准              |

---

## 5. 快速开始

### 5.1 环境要求

- CMake 3.12+
- C++11或更高
- Qt 5.9及以上

### 5.2 CMake 配置

在顶层 `CMakeLists.txt` 中配置 Qt SDK 路径：

```cmake
SET(QT_SDK_DIR D:/Qt/5.15.2/msvc2019_64 CACHE PATH "QT SDK DIR" FORCE)
```

### 5.3 引入头文件

```cpp
#include "ElaMetaType.h"
```

---

## 6. 定义可序列化类型

### 6.1 结构体

```cpp
struct BaseData
{
    QString name{"默认名称"};
    int age{0};
};
ELA_BEGIN_META(BaseData)          // 开始类型注册
ELA_FIELD(name, "姓名")           // 注册字段 name，描述为"姓名"
ELA_FIELD(age, "年龄")            // 注册字段 age，描述为"年龄"
ELA_END_META()                    // 结束类型注册

// 等价批量注册写法：
struct BaseData
{
    QString name{"默认名称"};
    int age{0};
};
ELA_BEGIN_META(BaseData)
ELA_FIELDS(name, age)             // 批量注册，描述默认为 "Field"
ELA_END_META()
```

### 6.2 QObject 派生类

```cpp
class MainData : public QWidget
{
    Q_OBJECT
    ELA_META_FRIEND(MainData)     // 友元声明，允许框架访问私有成员

public:
    explicit MainData(QWidget* parent = nullptr) : QWidget(parent) {}
    ~MainData() override = default;

private:
    QString title{"主窗口"};
    int width{800};
};
ELA_BEGIN_QT_META(MainData)       // QT_META 版本（要求 QObject 基类）
ELA_FIELD(title, "标题")
ELA_FIELD(width, "宽度")
ELA_END_META()
```

> `ELA_BEGIN_META` 用于普通结构体，`ELA_BEGIN_QT_META` 用于 QObject 派生类（含静态断言校验）。

### 6.3 枚举

```cpp
enum TestEnum
{
    Invalid = 0x0000,
    Valid   = 0x0010
};
ELA_ENUM_META(TestEnum)           // 注册枚举，序列化时以名称字符串存取
```

---

## 7. 继承

### 7.1 单继承

```cpp
struct BaseData
{
    QString baseName{"基类"};
};
ELA_BEGIN_META(BaseData)
ELA_FIELD(baseName, "基类名称")
ELA_END_META()

struct ChildData : BaseData
{
    QString childName{"子类"};
};
ELA_BEGIN_META(ChildData, BaseData)    // 第二个参数起为基类列表
ELA_FIELD(childName, "子类名称")
ELA_END_META()
```

`ChildData` 序列化时自动包含 `BaseData` 的所有字段，无需重复声明。

### 7.2 多重继承

```cpp
struct ChildData : BaseData1, BaseData2
{
    int member{0};
};
ELA_BEGIN_META(ChildData, BaseData1, BaseData2)  // 列出所有基类
ELA_FIELD(member, "成员")
ELA_END_META()
```

### 7.3 菱形继承

当多个基类共享同一个祖先类时，框架自动进行字段去重，同一字段在最终类型中只出现一次。

---

## 8. 函数字段

对于不直接暴露为成员变量或需要计算的值，使用 `ELA_FUNC_FIELD`：

```cpp
class MainData : public QWidget
{
    Q_OBJECT
    ELA_META_FRIEND(MainData)

public:
    void setTitle(const QString& t) { _title = t; }
    QString getTitle() const { return _title; }

private:
    QString _title;
};
ELA_BEGIN_QT_META(MainData)
ELA_FUNC_FIELD("标题", getTitle, setTitle)  // 描述 + getter + setter
ELA_END_META()
```

> getter 和 setter 的参数类型必须一致，编译期会进行静态断言检查。

---

## 9. 序列化与反序列化

### 9.1 XML 文本（可读）

```cpp
// 序列化
MainData* source = new MainData();
source->title = "Hello";

QString xml = ElaMetaType::getInstance()->serialize(source);
// 或使用泛型接口
QString xml = ElaMetaType::getInstance()->serialize<MainData*>(source);

// 反序列化
MainData* restored = ElaMetaType::getInstance()->deserialize<MainData*>(xml);
```

### 9.2 二进制（高速）

```cpp
// 序列化 — 不压缩
QByteArray bytes = ElaMetaType::getInstance()->serializeToByte(source, false);

// 序列化 — LZ4 压缩
QByteArray compressed = ElaMetaType::getInstance()->serializeToByte(source, true);

// 反序列化（自动检测压缩并解压）
MainData* restored = ElaMetaType::getInstance()->deserializeFromByte<MainData*>(bytes);
```

### 9.3 直接使用 ElaVariant

```cpp
ElaVariant var = ElaVariant::fromValue(source);
QString xml = ElaMetaType::getInstance()->serialize(var);

ElaVariant restoredVar = ElaMetaType::getInstance()->deserialize(xml);
MainData* restored = restoredVar.value<MainData*>();
```

---

## 10. 文件读写

```cpp
// 保存对象到 XML 文件
ElaMetaType::getInstance()->saveToFile("./Config.xml", source);

// 保存 XML 字符串到文件
ElaMetaType::saveToFile("./Data.xml", xmlString);

// 保存二进制数据到文件
QByteArray bytes = ElaMetaType::getInstance()->serializeToByte(source, true);
ElaMetaType::saveToFile("./Data.bin", bytes);

// 从文件读取 — XML 文本
MainData* data = ElaMetaType::getInstance()
    ->readFromFile("./Config.xml", false)    // isByte = false
    .value<MainData*>();

// 从文件读取 — 二进制
MainData* data = ElaMetaType::getInstance()
    ->readFromFile("./Data.bin", true)       // isByte = true
    .value<MainData*>();
```

---

## 11. 支持的容器类型

### 11.1 标准容器

所有标准容器直接作为字段即可，无需额外配置：

```cpp
struct ContainerDemo
{
    QList<int> intList{1, 2, 3};
    QMap<QString, ChildData> dataMap;
    std::vector<std::string> stringVec;
    std::map<int, ChildData> stdMap;
    std::tuple<QString, int, ChildData> tuple;
    QPair<QString, ChildData> pair;
    QQueue<int> queue;
};
ELA_BEGIN_META(ContainerDemo)
ELA_FIELDS(intList, dataMap, stringVec, stdMap, tuple, pair, queue)
ELA_END_META()
```

### 11.2 嵌套容器

支持任意深度的嵌套：

```cpp
QMap<QString, QList<ChildData>> nested1;           // Map → List → 自定义类型
std::vector<QMap<QString, QList<ChildData>>> nested2; // Vector → Map → List
```

### 11.3 ElaVariantList（异构容器）

可以存放任意不同类型的元素：

```cpp
ElaVariantList list;
list.append(ElaVariant::fromValue(1));
list.append(ElaVariant::fromValue(QString("字符串")));
list.append(ElaVariant::fromValue(childData));
```

---

## 12. 指针与智能指针

```cpp
struct PtrDemo
{
    ChildData* rawPtr{nullptr};                // 裸指针（Heap 模式）
    std::shared_ptr<ChildData> sharedPtr;      // 智能指针
    ChildData stackValue;                      // 栈值（Stack 模式）
};
ELA_BEGIN_META(PtrDemo)
ELA_FIELDS(rawPtr, sharedPtr, stackValue)
ELA_END_META()
```

| 声明方式                     | 反序列化行为                        |
|--------------------------|-------------------------------|
| `T value`                | 栈对象，值拷贝                       |
| `T* ptr`                 | heap 模式，自动 `new T()` 创建       |
| `std::shared_ptr<T> ptr` | 自动 `std::make_shared<T>()` 创建 |

> 反序列化时若指针为 `nullptr`，框架会自动构造默认对象。

---

## 13. 类型检查

```cpp
ElaMetaTypeHash hash = getMetaTypeHash<ChildData>();
if (ElaMetaType::getInstance()->isMetaTypeRegistered(hash))
{
    // 类型已注册，可安全序列化
}
```

---

## 14. 注意事项

1. **必须注册**：序列化前目标类型必须通过 `ELA_BEGIN_META` / `ELA_BEGIN_QT_META` 完成注册，否则序列化返回空结果并输出调试警告。

2. **QObject 派生类**：必须使用 `ELA_BEGIN_QT_META`，并在类体内添加 `ELA_META_FRIEND(ClassName)` 友元声明。

3. **字段描述**：`ELA_FIELD` 的第二个参数为字段的中文描述，会写入序列化输出。`ELA_FIELDS` 批量注册时描述为 "Field"。

4. **Qt 版本**：应对 Qt5/Qt6 的 `QPair`/`QVector` 差异做了兼容处理。

5. **二进制 vs XML**：二进制格式不可读但速度极快；XML 格式可读但速度较慢。根据场景选择合适通道。