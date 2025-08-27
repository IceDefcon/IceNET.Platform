/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <QSerialPortInfo>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSerialPort>
#include <QLineEdit>
#include <QDateTime>
#include <QComboBox>
#include <QString>
#include <QWidget>
#include <QDebug>
#include <QLabel>


#include <iostream>
#include <cstdint>

#include "KernelCtrl.h"
#include "KernelCommander.h"

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

class MainGui : public QWidget
{
    Q_OBJECT

    QPlainTextEdit *m_mainConsoleOutput;
    QPlainTextEdit *m_uartConsoleOutput;

    std::shared_ptr<std::vector<uint8_t>> m_Rx_MainGuiVector;
    std::shared_ptr<std::vector<uint8_t>> m_Tx_MainGuiVector;
    std::shared_ptr<KernelCommanderStateType> m_IO_MainGuiState;

    std::unique_ptr<KernelCtrl> m_instanceKernelCtrl;

    QSerialPort *m_serialPort;
    QByteArray m_readBuffer;
    QString m_uartPortName;
    QLineEdit *m_uartInput;
    bool m_uartIsConnected;
    QComboBox *m_serialPortDropdown;
    QLineEdit *m_baudrate_dataField;

    QString m_currentTime;

private slots:



public:

    MainGui();
    ~MainGui();

private slots:

    void setupWindow();
    void setupMainConsole();
    void setupUartControl();
    void setupUartConsole();
    void clearUartConsole();
    void refreshSerialPorts();

    void openUart();
    void readUartData();
    void writeToUart(const QString &data);
    void onUartInput();
    void shutdownUart();

    void printToMainConsole(const QString &message);
    void printToUartConsole(const QString &message);


    void setupNetworkControl();
    void setupMainCtrl();
    void setupSeparators();

    void createMainCtrl();
    void deleteMainCtrl();
};
