#include "CameraWindow.h"

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

    if (m_pipeline)
    {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
    }

    delete m_cameraDisplay;
    delete m_videoTimer;
}

void CameraWindow::updateCameraFrame()
{
    // Check if the camera is opened
    if (!m_cap.isOpened()) {
        qWarning() << "[CAM] Camera not opened!";
        return;
    }

    // Capture the frame from the camera
    cv::Mat frame;
    if (!m_cap.read(frame)) {
        qWarning() << "[CAM] Failed to read frame from camera.";
        return;
    }

    // Convert frame from BGR to RGB (for proper display in Qt and GStreamer)
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    // Convert the OpenCV frame to QImage for local display
    QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    m_cameraDisplay->setPixmap(QPixmap::fromImage(image).scaled(m_cameraDisplay->size(), Qt::KeepAspectRatio));

    // Initialize GStreamer once, after first valid frame
    static bool gst_initialized = false;
    if (!gst_initialized) {
        qDebug() << "[CAM] Initializing GStreamer...";
        initializeGStreamer();

        // Set up the GStreamer source element
        if (m_source) {
            qDebug() << "[CAM] Setting caps for GStreamer source with resolution:" << frame.cols << "x" << frame.rows;
            GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                                "format", G_TYPE_STRING, "RGB",
                                                "width", G_TYPE_INT, frame.cols,
                                                "height", G_TYPE_INT, frame.rows,
                                                "framerate", GST_TYPE_FRACTION, 30, 1,
                                                nullptr);
            g_object_set(m_source, "caps", caps, nullptr);
            gst_caps_unref(caps);
        } else {
            qWarning() << "[CAM] GStreamer source element is null!";
        }

        gst_initialized = true;
        qDebug() << "[CAM] GStreamer initialized.";
    }

    // Push frame to GStreamer if the source element is valid
    if (m_source) {
        qDebug() << "[CAM] Preparing to push frame to GStreamer.";

        // Create a GstBuffer for the frame
        GstBuffer *buffer = gst_buffer_new_allocate(nullptr, frame.total() * frame.elemSize(), nullptr);
        if (!buffer) {
            qWarning() << "[CAM] Failed to allocate GstBuffer.";
            return;
        }

        // Map the buffer and copy frame data into it
        GstMapInfo map;
        if (!gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
            qWarning() << "[CAM] Failed to map GstBuffer for writing.";
            gst_buffer_unref(buffer);
            return;
        }

        // Copy the frame data to the buffer
        memcpy(map.data, frame.data, frame.total() * frame.elemSize());
        gst_buffer_unmap(buffer, &map);

        qDebug() << "[CAM] GstBuffer prepared with size:" << frame.total() * frame.elemSize() << "bytes.";

        // Push the buffer to the GStreamer pipeline
        GstFlowReturn ret;
        g_signal_emit_by_name(m_source, "push-buffer", buffer, &ret);
        gst_buffer_unref(buffer);

        if (ret != GST_FLOW_OK) {
            qWarning() << "[CAM] GStreamer push-buffer failed with code:" << ret;
        } else {
            qDebug() << "[CAM] Successfully pushed frame:" << frame.cols << "x" << frame.rows;
        }
    } else {
        qWarning() << "[CAM] GStreamer source element is not initialized or is null!";
    }
}

void CameraWindow::initializeGStreamer()
{
    gst_init(nullptr, nullptr);

    // Create GStreamer elements
    m_source    = gst_element_factory_make("appsrc",     "source");
    m_encoder   = gst_element_factory_make("x264enc",    "encoder");
    m_payloader = gst_element_factory_make("rtph264pay", "payloader");
    m_sink      = gst_element_factory_make("udpsink",    "sink");
    m_pipeline  = gst_pipeline_new("video-pipeline");

    if (!m_pipeline || !m_source || !m_encoder || !m_payloader || !m_sink) {
        qWarning() << "[CAM] Failed to create GStreamer elements.";
        return;
    }

    // Configure udpsink
    g_object_set(m_sink,
                 "host", "192.168.8.101",
                 "port", 5000,
                 "async", FALSE,
                 "sync", FALSE,
                 nullptr);

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), m_source, m_encoder, m_payloader, m_sink, nullptr);

    // Link elements in order
    if (!gst_element_link_many(m_source, m_encoder, m_payloader, m_sink, nullptr)) {
        qWarning() << "[CAM] Failed to link GStreamer elements.";
        return;
    }

    // Set pipeline state
    GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "[CAM] Failed to set pipeline to PLAYING.";
        return;
    }

    qDebug() << "[CAM] GStreamer pipeline initialized and running.";
}
