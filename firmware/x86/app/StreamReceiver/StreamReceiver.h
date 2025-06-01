#ifndef STREAMRECEIVER_H
#define STREAMRECEIVER_H

#include <QLabel>
#include <QTimer>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <opencv2/opencv.hpp>

class StreamReceiver : public QLabel
{
    Q_OBJECT

public:
    explicit StreamReceiver(QWidget* parent = nullptr);
    ~StreamReceiver();

private slots:
    void pullFrameFromGStreamer();

private:
    QTimer* m_frameTimer;
    GstElement* m_pipeline = nullptr;
    GstElement* m_sink = nullptr;
};

#endif // STREAMRECEIVER_H
