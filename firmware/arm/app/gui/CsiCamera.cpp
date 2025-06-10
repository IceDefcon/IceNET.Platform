/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "CsiCamera.h"

CsiCamera::CsiCamera() :
    m_cameraDisplay(new QLabel(this)),
    m_videoTimer(new QTimer(this))
{
    qDebug() << "[CAM] [CONSTRUCTOR]" << this << "::  CsiCamera";

    setWindowTitle("Camera Feed");
    resize(640, 480);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_cameraDisplay);
    setLayout(layout);

    m_cameraDisplay->setAlignment(Qt::AlignCenter);
    m_cameraDisplay->setStyleSheet("background-color: black");

    m_cap.open("/dev/video0", cv::CAP_V4L2);

    if (!m_cap.isOpened())
    {
        qWarning() << "[CAM] Failed to open camera.";
        return;
    }

    /**
     *
     * v4l2-ctl --device=/dev/video0 --all
     * v4l2-ctl --device=/dev/video0 --list-formats-ext
     * v4l2-ctl --device=/dev/video0 --stream-mmap=3 --stream-to=test0.raw --stream-count=1
     * v4l2-ctl --device=/dev/video0 --get-fmt-video
     *
     * Pixel Format: 'RG10'
     * Name: 10-bit Bayer RGRG/GBGB
     * Size: Discrete 3264x2464 :: Interval: Discrete 0.048s (21.000 fps)
     * Size: Discrete 3264x1848 :: Interval: Discrete 0.036s (28.000 fps)
     * Size: Discrete 1920x1080 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 1640x1232 :: Interval: Discrete 0.033s (30.000 fps)
     * Size: Discrete 1280x720 :: Interval: Discrete 0.017s (60.000 fps)
     *
     */
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('R', 'G', '1', '0'));

    double actualWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double actualHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    qDebug() << "[CAM] Actual camera resolution:" << actualWidth << "x" << actualHeight;

    int fourcc = m_cap.get(cv::CAP_PROP_FOURCC);
    qDebug() << "[CAM] Camera FourCC: " << fourcc;

    /* GStreamer pipeline */
    std::string gstPipeline =
    "appsrc ! " // Source of the video stream
    "videoconvert ! " // Convert the video format if needed
    "x264enc tune=zerolatency bitrate=500 speed-preset=ultrafast ! " // H.264 encoding
    "rtph264pay config-interval=1 pt=96 ! " // RTP packetization
    "udpsink host=192.168.8.101 port=5000"; // UDP sink to the specified IP and port

    /* Open GStreamer writer */
    m_writer.open(gstPipeline, 0, 30, cv::Size(1280, 720), true);

    if (!m_writer.isOpened())
    {
        qWarning() << "[CAM] Failed to open GStreamer pipeline.";
    }

    connect(m_videoTimer, &QTimer::timeout, this, &CsiCamera::updateCameraFrame);
    m_videoTimer->start(16);
}

CsiCamera::~CsiCamera()
{
    qDebug() << "[CAM] [DESTRUCTOR]" << this << ":: CsiCamera ";

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

void CsiCamera::updateCameraFrame()
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

