/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <opencv2/opencv.hpp>
#include <QVBoxLayout>
#include <QPixmap>
#include <QDialog>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QTimer>

class UsbCamera : public QDialog
{
    Q_OBJECT

public:
    UsbCamera();
    ~UsbCamera();

private slots:
    void updateCameraFrame();

private:
    QLabel* m_cameraDisplay;
    QTimer* m_videoTimer;
    cv::VideoCapture m_cap;
    cv::VideoWriter m_writer;  // <<--- Added for GStreamer
};
