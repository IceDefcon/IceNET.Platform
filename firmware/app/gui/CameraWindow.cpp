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
    m_cap.open("/dev/video0");

    if (!m_cap.isOpened())
    {
        qWarning() << "[CameraWindow] Failed to open camera.";
        return;
    }

    /**
     * v4l2-ctl --list-formats-ext
     *
     * Pixel Format: 'MJPG' (compressed)
     * Name        : Motion-JPEG
     * Size: Discrete 1280x720 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 640x480 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 320x240 :: Interval: Discrete 0.033s (30.000 fps)
     *
     * Pixel Format: 'YUYV'
     * Name        : YUYV 4:2:2
     * Size: Discrete 1280x720 :: Interval: Discrete 0.100s (10.000 fps)
     * Size: Discrete 640x480 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 320x240 :: Interval: Discrete 0.033s (30.000 fps)
     *
     */
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    /**
     * By default we have:
     *
     * m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
     *
     */
    m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

    double actualWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double actualHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    qDebug() << "[CameraWindow] Actual camera resolution:" << actualWidth << "x" << actualHeight;

    int fourcc = m_cap.get(cv::CAP_PROP_FOURCC);
    qDebug() << "Camera FourCC: " << fourcc;

    double fps = m_cap.get(cv::CAP_PROP_FPS);
    qDebug() << "Camera FPS: " << fps;

    /* GStreamer pipeline */
    std::string gstPipeline = "appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=ultrafast "
        "! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.8.101 port=5000";

    /* Open GStreamer writer */
    m_writer.open(gstPipeline, 0, 30, cv::Size(1280, 720), true);

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
        cv::resize(frame, frame, cv::Size(1280, 720));

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
