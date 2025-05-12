#ifndef CAMERA_WINDOW_H
#define CAMERA_WINDOW_H

#include <opencv2/opencv.hpp>
#include <QVBoxLayout>
#include <gst/gst.h>
#include <QPixmap>
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QImage>

class CameraWindow : public QDialog
{
    Q_OBJECT

public:
    CameraWindow();
    ~CameraWindow();

private slots:
    void updateCameraFrame();
    void initializeGStreamer();

private:
    QLabel* m_cameraDisplay;
    QTimer* m_videoTimer;
    cv::VideoCapture m_cap;

    GstElement *m_pipeline = nullptr;
    GstElement *m_source = nullptr;
    GstElement *m_encoder = nullptr;
    GstElement *m_payloader = nullptr;
    GstElement *m_sink = nullptr;

    GstBus *m_bus = nullptr;
};

#endif // CAMERA_WINDOW_H
