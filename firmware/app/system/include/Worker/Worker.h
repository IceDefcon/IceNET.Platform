/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <QObject>
#include <QThread>
#include <QDebug>

#pragma once

class Worker : public QObject
{
    Q_OBJECT

public:

    explicit Worker(QObject *parent = nullptr);
    ~Worker();

public slots:
    void doWork();

signals:
    void workFinished();
};
