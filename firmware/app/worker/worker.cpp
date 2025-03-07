#include "worker.h"
#include "DroneCtrl.h"

worker::worker()
{

}

void worker::doWork()
{
    qDebug() << "Working in thread: " << QThread::currentThread();

    DroneCtrl ctrl;
    ctrl.applicationStart();

    qDebug() << "Work finished";
    emit workFinished(); // Emit signal when work is finished
}
