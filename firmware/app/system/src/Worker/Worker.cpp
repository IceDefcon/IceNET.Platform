/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "Worker.h"
#include "DroneCtrl.h"

Worker::Worker(QObject *parent) : QObject(parent) {}

Worker::~Worker() {}

void Worker::doWork()
{
    qDebug() << "Working in thread: " << QThread::currentThread();

    DroneCtrl ctrl;
    ctrl.applicationStart();

    qDebug() << "Work finished";
    emit workFinished(); // Emit signal when work is finished
}
