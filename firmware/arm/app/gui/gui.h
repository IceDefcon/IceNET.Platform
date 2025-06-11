/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <opencv2/opencv.hpp>
#include <QSerialPortInfo>
#include <QPlainTextEdit>
#include <QApplication>
#include <QSerialPort>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QLineEdit>
#include <QCheckBox>
#include <QPainter>
#include <QPixmap>
#include <QDialog>
#include <QWidget>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QTimer>
#include <QFont>

#include <thread>
#include <mutex>
#include <cmath>

#include "DroneCtrl.h"
#include "CsiCamera.h"
#include "UsbCamera.h"

typedef struct
{
    uint32_t xWindow;
    uint32_t yWindow;
    uint32_t xGap;
    uint32_t yGap;
    uint32_t xLogo;
    uint32_t yLogo;
    uint32_t xText;
    uint32_t xUnit;
    uint32_t yUnit;
    uint32_t separatorLength;
    uint32_t separatorWidth;
} mainWindowType;

typedef struct
{
    uint32_t xPosition;
    uint32_t yPosition;
    uint32_t xSize;
    uint32_t ySize;
} mainConsoleType;

class gui : public QWidget
{
    Q_OBJECT

    QPlainTextEdit *m_mainConsoleOutput;
    QPlainTextEdit *m_uartConsoleOutput;

    QLineEdit *m_i2c_addressField;
    QLineEdit *m_i2c_registerField;
    QLineEdit *m_i2c_burstField;
    QLineEdit *m_i2c_dataField;
    QCheckBox *m_i2c_writeTick;

    QLineEdit *m_spi_addressField;
    QLineEdit *m_spi_registerField;
    QLineEdit *m_spi_burstField;
    QLineEdit *m_spi_dataField;
    QCheckBox *m_spi_writeTick;

    QLineEdit *m_ext_Byte_0;
    QLineEdit *m_ext_Byte_1;
    QLineEdit *m_ext_Byte_2;
    QCheckBox *m_ext_writeTick;

    QLineEdit *m_pwm_dataField;
    QLineEdit *m_dmaCustom_dataField;

    QPushButton *m_enableButton;
    QPushButton *m_startButton;
    QPushButton *m_debugButton;

    std::unique_ptr<DroneCtrl> m_instanceDroneControl;
    bool m_isKernelConnected;

    std::shared_ptr<std::vector<uint8_t>> m_Rx_GuiVector;
    std::shared_ptr<std::vector<uint8_t>> m_Tx_GuiVector;
    std::shared_ptr<ioStateType> m_IO_GuiState;

    QSerialPort *m_serialPort;
    QByteArray m_readBuffer;
    QString m_uartPortName;
    QLineEdit *m_uartInput;
    bool m_uartIsConnected;

    QString m_currentTime;

    bool m_isPulseControllerEnabled;
    bool m_isStartAcquisition;
    bool m_isDebugEnabled;

    QLabel *m_cameraDisplay;
    QTimer *m_videoTimer;
    cv::VideoCapture m_cap;

    CsiCamera* m_CsiCamera;
    UsbCamera* m_UsbCamera;

    QPushButton *m_cameraButton;

private slots:

    /* GET */ const mainWindowType* getMainWindow();
    /* GET */ const mainConsoleType* getMainConsole();

public:

    gui();
    ~gui();

private slots:

    void setupWindow();
    void setupMainConsole();
    void setupUartConsole();

    void setupFpgaCtrl();
    void setupDroneControl();
    void setupSeparators();
    void setupCameras();
    void setupAdditionalDebugs();

    void setupI2C();
    void setupSPI();
    void setupPWM();
    void setupDMA();
    void setupCMD();
    void setupEXT();

    void setDeadCommand();
    void setDummyCommand();
    void setInterruptVector(uint8_t vector);
    std::string vectorToString(interruptVectorType type);
    void interruptVector_execute(interruptVectorType intVector);
    std::string cmdToString(commandType cmd);
    void dma_execute(commandType cmd);
    void i2c_execute();
    void spi_execute();
    void ext_execute();
    void pwm_execute(pwmType type);

    void openUart();
    void readUartData();
    void writeToUart(const QString &data);
    void onUartInput();
    void shutdownUart();

    void printToMainConsole(const QString &message);
    void printToUartConsole(const QString &message);

    void createDroneControl();
    void deleteDroneControl();

    void C1_Execute();
    void C2_Execute();
    void C3_Execute();
    void C4_Execute();

    void openCsiCamera();
    void openUsbCamera();
};
