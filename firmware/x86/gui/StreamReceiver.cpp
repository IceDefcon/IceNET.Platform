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

    // Initialize GStreamer
    gst_init(nullptr, nullptr);

    // Build GStreamer pipeline
    const char* pipelineDesc =
        "udpsrc port=5000 caps=\"application/x-rtp, media=video, encoding-name=H264, payload=96\" ! "
        "rtph264depay ! avdec_h264 ! videoconvert ! appsink name=sink";

    GError* error = nullptr;
    m_pipeline = gst_parse_launch(pipelineDesc, &error);

    if (!m_pipeline) {
        qWarning() << "Failed to create GStreamer pipeline:" << error->message;
        g_error_free(error);
        return;
    }

    m_sink = gst_bin_get_by_name(GST_BIN(m_pipeline), "sink");

    // Configure appsink
    g_object_set(m_sink, "emit-signals", FALSE, "sync", FALSE, nullptr);
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

    connect(m_frameTimer, &QTimer::timeout, this, &StreamReceiver::pullFrameFromGStreamer);
    m_frameTimer->start(30); // ~33ms ~ 30 FPS
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
    if (!m_sink) return;

    GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(m_sink), 0);
    if (!sample) return;

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstCaps* caps = gst_sample_get_caps(sample);
    GstStructure* s = gst_caps_get_structure(caps, 0);

    int width, height;
    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return;
    }

    cv::Mat frame(height, width, CV_8UC3, (char*)map.data, cv::Mat::AUTO_STEP);
    QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    setPixmap(QPixmap::fromImage(image.copy()).scaled(size(), Qt::KeepAspectRatio)); // Use copy() to avoid dangling pointer

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
}
