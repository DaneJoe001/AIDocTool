#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序图标
    app.setWindowIcon(QIcon(":/icons/main_icon.ico"));
    
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
} 