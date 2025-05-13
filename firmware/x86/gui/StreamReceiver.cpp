#include "StreamReceiver.h"
#include <QImage>
#include <QPixmap>
#include <QDebug>

StreamReceiver::StreamReceiver(QWidget* parent)
    : QLabel(parent), m_frameTimer(new QTimer(this))
{
    setWindowTitle("Remote Camera Feed");
    resize(640, 480);
    setAlignment(Qt::AlignCenter);
    setStyleSheet("background-color: black");

    // Initialize GStreamer
    gst_init(nullptr, nullptr);

    // GStreamer pipeline with enforced RGB format
    const char* pipelineDesc =
        "udpsrc port=5000 caps=\"application/x-rtp, media=video, encoding-name=H264, payload=96\" ! "
        "rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw,format=RGB ! appsink name=sink";

    GError* error = nullptr;
    m_pipeline = gst_parse_launch(pipelineDesc, &error);

    if (!m_pipeline) {
        qWarning() << "GStreamer: Failed to create pipeline:" << (error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        return;
    }

    m_sink = gst_bin_get_by_name(GST_BIN(m_pipeline), "sink");
    if (!m_sink) {
        qWarning() << "GStreamer: Failed to get appsink element.";
        return;
    }

    // Configure appsink
    g_object_set(m_sink,
                 "emit-signals", FALSE,
                 "sync", FALSE,
                 "max-buffers", 1,
                 "drop", TRUE,
                 nullptr);

    // Set pipeline to PLAYING
    GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "GStreamer: Failed to start pipeline.";
        return;
    }

    connect(m_frameTimer, &QTimer::timeout, this, &StreamReceiver::pullFrameFromGStreamer);
    m_frameTimer->start(30); // ~30 FPS
}

StreamReceiver::~StreamReceiver()
{
    if (m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(m_pipeline));
    }
    if (m_sink) {
        gst_object_unref(m_sink);
    }
}

void StreamReceiver::pullFrameFromGStreamer()
{
    if (!m_sink) {
        qWarning() << "No sink available.";
        return;
    }

    // Try to pull a sample with a short timeout
    GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(m_sink), 500000); // 0.5s timeout
    if (!sample) {
        qDebug() << "[Receiver] No sample received from appsink.";
        return;
    }

    // Get buffer and caps
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstCaps* caps = gst_sample_get_caps(sample);
    if (!buffer || !caps) {
        qWarning() << "[Receiver] Invalid buffer or caps.";
        gst_sample_unref(sample);
        return;
    }

    GstStructure* s = gst_caps_get_structure(caps, 0);

    int width = 0, height = 0;
    const gchar* format = gst_structure_get_string(s, "format");

    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);

    qDebug() << "[Receiver] Frame:" << width << "x" << height << "Format:" << format;

    if (width == 0 || height == 0 || !format || strcmp(format, "RGB") != 0) {
        qWarning() << "[Receiver] Unexpected frame format or size.";
        gst_sample_unref(sample);
        return;
    }

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        qWarning() << "[Receiver] Failed to map buffer.";
        gst_sample_unref(sample);
        return;
    }

    // Build OpenCV frame and convert to QImage
    cv::Mat frame(height, width, CV_8UC3, (char*)map.data, cv::Mat::AUTO_STEP);

    // Use a copy of the image to avoid using dangling memory
    QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    setPixmap(QPixmap::fromImage(image.copy()).scaled(size(), Qt::KeepAspectRatio));

    // Cleanup
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
}
