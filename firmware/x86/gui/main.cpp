#include <QApplication>
#include "StreamReceiver.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    StreamReceiver receiver;
    receiver.show();
    return app.exec();
}
