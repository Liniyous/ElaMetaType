
#include "MainWindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow mainWindow;
    return QApplication::exec();
}
