#ifndef STREAMRECEIVER_H
#define STREAMRECEIVER_H

#include <QLabel>
#include <QUdpSocket>
#include <opencv2/opencv.hpp>

class StreamReceiver : public QLabel
{
    Q_OBJECT

public:
    explicit StreamReceiver(QWidget* parent = nullptr);

private slots:
    void readPendingDatagrams();

private:
    QUdpSocket* m_socket;
};

#endif // STREAMRECEIVER_H
