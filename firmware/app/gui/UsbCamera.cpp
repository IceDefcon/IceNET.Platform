/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "UsbCamera.h"

UsbCamera::UsbCamera() :
    m_cameraDisplay(new QLabel(this)),
    m_videoTimer(new QTimer(this))
{
    qDebug() << "[CAM] [CONSTRUCTOR]" << this << "::  UsbCamera";

    setWindowTitle("Camera Feed");
    resize(640, 480);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_cameraDisplay);
    setLayout(layout);

    m_cameraDisplay->setAlignment(Qt::AlignCenter);
    m_cameraDisplay->setStyleSheet("background-color: black");

    m_cap.open("/dev/video1", cv::CAP_V4L2);

    if (!m_cap.isOpened())
    {
        qWarning() << "[CAM] Failed to open camera.";
        return;
    }

    /**
     * v4l2-ctl --device=/dev/video1 --all
     * v4l2-ctl --device=/dev/video1 --list-formats-ext
     * v4l2-ctl --device=/dev/video1 --stream-mmap=3 --stream-to=test0.raw --stream-count=1
     * v4l2-ctl --device=/dev/video1 --get-fmt-video
     *
     * Pixel Format: 'MJPG' (compressed)
     * Name: Motion-JPEG
     * Size: Discrete 1280x720 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 640x480 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 320x240 :: Interval: Discrete 0.033s (30.000 fps)
     *
     * Pixel Format: 'YUYV'
     * Name: YUYV 4:2:2
     * Size: Discrete 1280x720 :: Interval: Discrete 0.100s (10.000 fps)
     * Size: Discrete 640x480 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 320x240 :: Interval: Discrete 0.033s (30.000 fps)
     *
     */
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

    double actualWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double actualHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    qDebug() << "[CAM] Actual camera resolution:" << actualWidth << "x" << actualHeight;

    int fourcc = m_cap.get(cv::CAP_PROP_FOURCC);
    qDebug() << "[CAM] Camera FourCC: " << fourcc;

    double fps = m_cap.get(cv::CAP_PROP_FPS);
    qDebug() << "[CAM] Camera FPS: " << fps;

    /* GStreamer pipeline */
    std::string gstPipeline = "appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=ultrafast "
        "! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.8.101 port=5000";

    /* Open GStreamer writer */
    m_writer.open(gstPipeline, 0, 30, cv::Size(1280, 720), true);

    if (!m_writer.isOpened())
    {
        qWarning() << "[CAM] Failed to open GStreamer pipeline.";
    }

    connect(m_videoTimer, &QTimer::timeout, this, &UsbCamera::updateCameraFrame);
    m_videoTimer->start(33); /* 30 FPS -> 1/30 = 0.0333 */
}

UsbCamera::~UsbCamera()
{
    qDebug() << "[CAM] [DESTRUCTOR]" << this << ":: UsbCamera ";

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

void UsbCamera::updateCameraFrame()
{
    if (!m_cap.isOpened()) return;

    cv::Mat frame;
    if (m_cap.read(frame))
    {
        /* Force resize to 640x480 before sending */
        cv::resize(frame, frame, cv::Size(1280, 720));

        qDebug() << "[CAM] Sending frame:" << frame.cols << "x" << frame.rows;

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
