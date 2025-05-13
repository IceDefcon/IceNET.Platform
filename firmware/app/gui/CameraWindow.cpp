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

    /* Open the default camera -> /dev/video0 */
    m_cap.open(0);

    if (!m_cap.isOpened())
    {
        qWarning() << "[CameraWindow] Failed to open camera.";
        return;
    }

    /* Set a lower resolution -> 320x240, 640x480 */
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    double actualWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double actualHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    qDebug() << "[CameraWindow] Actual camera resolution:" << actualWidth << "x" << actualHeight;

    /* GStreamer pipeline */
    std::string gstPipeline = "appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=ultrafast "
        "! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.8.101 port=5000";

    /* Open GStreamer writer */
    m_writer.open(gstPipeline, 0, 30, cv::Size(640, 480), true);

    if (!m_writer.isOpened())
    {
        qWarning() << "[CameraWindow] Failed to open GStreamer pipeline.";
    }

    connect(m_videoTimer, &QTimer::timeout, this, &CameraWindow::updateCameraFrame);
    m_videoTimer->start(33); /* 30 FPS -> 1/30 = 0.0333 */
}

CameraWindow::~CameraWindow()
{
    qDebug() << "[CAM] [DESTRUCTOR]" << this << ":: CameraWindow ";

    if (m_cap.isOpened())
    {
        m_cap.release();
    }

    if (m_writer.isOpened())
    {
        m_writer.release();
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
        /* Force resize to 640x480 before sending */
        cv::resize(frame, frame, cv::Size(640, 480));

        qDebug() << "[CameraWindow] Sending frame:" << frame.cols << "x" << frame.rows;

        /* Send to GStreamer */
        if (m_writer.isOpened())
        {
            m_writer.write(frame);
        }

        cv::Mat rgbFrame;
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
        QImage image(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
        m_cameraDisplay->setPixmap(QPixmap::fromImage(image).scaled(m_cameraDisplay->size(), Qt::KeepAspectRatio));
    }
}
