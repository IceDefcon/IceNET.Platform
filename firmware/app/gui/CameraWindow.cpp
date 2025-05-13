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

    // Define GStreamer pipeline
    std::string gstPipeline =
        "appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=ultrafast "
        "! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.8.101 port=5000";

    // Get the camera resolution
    int width = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Open the GStreamer writer
    m_writer.open(gstPipeline, 0, 30, cv::Size(width, height), true);

    if (!m_writer.isOpened())
    {
        qWarning() << "[CameraWindow] Failed to open GStreamer pipeline.";
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
        // Write to GStreamer pipeline
        if (m_writer.isOpened())
        {
            m_writer.write(frame);
        }

        // Display in Qt window
        cv::Mat rgbFrame;
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
        QImage image(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
        m_cameraDisplay->setPixmap(QPixmap::fromImage(image).scaled(m_cameraDisplay->size(), Qt::KeepAspectRatio));
    }
}
