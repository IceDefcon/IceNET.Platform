/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include "gui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    gui window;
    window.show();

    return app.exec();
}
