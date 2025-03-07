#include <QObject>
#include <QThread>
#include <QDebug>

class worker : public QObject
{
    Q_OBJECT

public:

    worker();

public slots:

    void doWork();

signals:
    void workFinished();
};
