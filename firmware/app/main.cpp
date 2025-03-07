#include "gui.h"
#include "worker.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    gui window;
    window.show();

    // Create the worker and thread
    worker *workerObj = new worker;
    QThread *thread = new QThread;

    // Move the worker to the thread
    workerObj->moveToThread(thread);

    // Connect signals and slots
    QObject::connect(thread, &QThread::started, workerObj, &worker::doWork);
    QObject::connect(workerObj, &worker::workFinished, thread, &QThread::quit);
    QObject::connect(workerObj, &worker::workFinished, workerObj, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // Start the thread
    thread->start();

    return app.exec();
}
