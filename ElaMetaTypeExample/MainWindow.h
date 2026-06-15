#ifndef TEST_MAINWINDOW_H_
#define TEST_MAINWINDOW_H_

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow() override;

private:
};

#endif //TEST_MAINWINDOW_H_
