#include "worker/worker.h"

worker::worker()
{

}

void worker::doWork()
{
    qDebug() << "Working in thread: " << QThread::currentThread();
    //
    // Wait
    //
    QThread::sleep(2);
    qDebug() << "Work finished";
    emit workFinished(); // Emit signal when work is finished
}
