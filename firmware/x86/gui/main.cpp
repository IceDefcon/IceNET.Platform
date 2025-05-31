#include <QApplication>
#include "mainCtrl.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    mainCtrl ctrl;
    ctrl.show();

    return app.exec();
}
