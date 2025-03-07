/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "gui.h"
#include "Worker.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    gui window;
    window.show();

    // Create the Worker and thread
    Worker *workerObj = new Worker;
    QThread *thread = new QThread;

    // Move the Worker to the thread
    workerObj->moveToThread(thread);

    // Connect signals and slots
    QObject::connect(thread, &QThread::started, workerObj, &Worker::doWork);
    QObject::connect(workerObj, &Worker::workFinished, thread, &QThread::quit);
    QObject::connect(workerObj, &Worker::workFinished, workerObj, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // Start the thread
    thread->start();

    return app.exec();
}
