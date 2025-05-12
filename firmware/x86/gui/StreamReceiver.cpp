#include "StreamReceiver.h"
#include <QImage>
#include <QPixmap>

StreamReceiver::StreamReceiver(QWidget* parent)
    : QLabel(parent), m_socket(new QUdpSocket(this))
{
    setWindowTitle("Remote Camera Feed");
    resize(640, 480);
    setAlignment(Qt::AlignCenter);

    m_socket->bind(5000, QUdpSocket::ShareAddress);  // Match sender port

    connect(m_socket, &QUdpSocket::readyRead, this, &StreamReceiver::readPendingDatagrams);
}

void StreamReceiver::readPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(int(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(datagram.data(), datagram.size());

        std::vector<uchar> buffer(datagram.begin(), datagram.end());
        cv::Mat bgrFrame = cv::imdecode(buffer, cv::IMREAD_COLOR);
        if (!bgrFrame.empty())
        {
            cv::Mat rgbFrame;
            cv::cvtColor(bgrFrame, rgbFrame, cv::COLOR_BGR2RGB);

            QImage qimg(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
            setPixmap(QPixmap::fromImage(qimg).scaled(size(), Qt::KeepAspectRatio));
        }
    }
}
