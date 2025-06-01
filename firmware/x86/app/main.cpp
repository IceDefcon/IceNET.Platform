#include <QApplication>
#include "MainGui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainGui ctrl;
    ctrl.show();

    return app.exec();
}
