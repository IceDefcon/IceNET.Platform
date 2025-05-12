#include "CameraWindow.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QImage>
#include <QPixmap>

CameraWindow::CameraWindow() :
    m_cameraDisplay(new QLabel(this)),
    m_videoTimer(new QTimer(this))
{
    qDebug() << "[CAM] [CONSTRUCTOR]" << this << "::  CameraWindow";

    setWindowTitle("Camera Feed");
    resize(640, 480);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_cameraDisplay);
    setLayout(layout);

    m_cameraDisplay->setAlignment(Qt::AlignCenter);
    m_cameraDisplay->setStyleSheet("background-color: black");

    // Open the default camera
    m_cap.open(0);

    if (!m_cap.isOpened())
    {
        qWarning() << "[CameraWindow] Failed to open camera.";
        return;
    }

    connect(m_videoTimer, &QTimer::timeout, this, &CameraWindow::updateCameraFrame);
    m_videoTimer->start(33); // ~30 FPS
}

CameraWindow::~CameraWindow()
{
    qDebug() << "[CAM] [DESTRUCTOR]" << this << ":: CameraWindow ";

    if (m_cap.isOpened())
    {
        m_cap.release();
    }

    delete m_cameraDisplay;
    delete m_videoTimer;
}

void CameraWindow::updateCameraFrame()
{
    if (!m_cap.isOpened()) return;

    cv::Mat frame;
    if (m_cap.read(frame))
    {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        m_cameraDisplay->setPixmap(QPixmap::fromImage(image).scaled(m_cameraDisplay->size(), Qt::KeepAspectRatio));
    }
}
