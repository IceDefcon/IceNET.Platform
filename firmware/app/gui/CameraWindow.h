#ifndef CAMERA_WINDOW_H
#define CAMERA_WINDOW_H

#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <opencv2/opencv.hpp>

class CameraWindow : public QDialog
{
    Q_OBJECT

public:
    CameraWindow();
    ~CameraWindow();

private slots:
    void updateCameraFrame();

private:
    QLabel* m_cameraDisplay;
    QTimer* m_videoTimer;
    cv::VideoCapture m_cap;
    cv::VideoWriter m_writer;  // <<--- Added for GStreamer
};

#endif // CAMERA_WINDOW_H
