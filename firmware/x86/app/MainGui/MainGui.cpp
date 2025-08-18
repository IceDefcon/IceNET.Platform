/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "MainGui.h"

static const mainWindowType w =
{
    .xWindow = 1200,
    .yWindow = 750,
    .xGap = 5,
    .yGap = 5,
    .xLogo = 200,
    .yLogo = 50,
    .xText = 140,
    .xUnit = 50,
    .yUnit = 25,
    .separatorLength = 500,
    .separatorWidth = 2,
};

static const mainConsoleType c =
{
    .xPosition = w.xGap*5 + w.xText + w.xUnit*2,  // Vertical Separator
    .yPosition = w.yGap*6 + w.yLogo + w.yUnit*3,  // At SPI Logo
    .xSize = w.xWindow - c.xPosition - w.xGap,    // Obvious
    .ySize = w.yGap*8 + w.yLogo*2 + w.yUnit*6+1,  // Last Horizontal Separator - yPosition + yGap
};

MainGui::MainGui() :
    m_Rx_MainGuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
    m_Tx_MainGuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
    m_IO_MainGuiState(std::make_shared<KernelCommanderStateType>(KERNEL_COMMANDER_IDLE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate MainGui" << std::endl;

    setupWindow();
    setupMainConsole();
    setupUartControl();
    setupUartConsole();
    setupNetworkControl();
    setupMainCtrl();
    setupSeparators();
}


MainGui::~MainGui()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy MainGui" << std::endl;
}

void MainGui::setupWindow()
{
    setWindowTitle("IceNET Platform");
    setFixedSize(w.xWindow, w.yWindow);
}

void MainGui::setupMainConsole()
{
    m_mainConsoleOutput = new QPlainTextEdit(this);
    m_mainConsoleOutput->setReadOnly(true);
    m_mainConsoleOutput->setGeometry(c.xPosition, c.yPosition, c.xSize - 400, c.ySize);
    m_mainConsoleOutput->setPlainText("[INIT] Main Console Initialized...");
}

void MainGui::printToMainConsole(const QString &message)
{
    m_mainConsoleOutput->appendPlainText(message);
}

void MainGui::setupUartControl()
{
    /* UART :: Row[0] */
    QLabel *uart_label = new QLabel("UART", this);
    QFont uart_labelFont;
    uart_labelFont.setPointSize(30);
    uart_labelFont.setItalic(true);
    uart_labelFont.setBold(true);
    uart_label->setFont(uart_labelFont);
    uart_label->setGeometry(w.xGap, w.yGap*16 + w.yLogo*3 + w.yUnit*9, w.xLogo, w.yLogo);

    m_serialPortDropdown = new QComboBox(this);
    m_serialPortDropdown->setGeometry(w.xGap, w.yGap*16 + w.yLogo*4 + w.yUnit*9, w.xText, w.yUnit);
    m_serialPortDropdown->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports)
    {
        m_serialPortDropdown->addItem(info.portName());
    }

    if (ports.isEmpty())
    {
        m_serialPortDropdown->addItem("No devices found");
        m_serialPortDropdown->setEnabled(false);
    }
    else
    {
        m_serialPortDropdown->setEnabled(true);
    }

    auto onSerialPortChanged = [this](int index)
    {
        QString port = m_serialPortDropdown->itemText(index);
        qDebug() << "Selected port:" << port;
    };
    connect(m_serialPortDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, onSerialPortChanged);

    QPushButton *uartOpen_exeButton = new QPushButton("OPEN", this);
    uartOpen_exeButton->setGeometry(w.xGap*3 + w.xText, w.yGap*16 + w.yLogo*4 + w.yUnit*9, w.xUnit*2, w.yUnit);

    auto openSelectedUart = [this]()
    {
        openUart();

    };
    connect(uartOpen_exeButton, &QPushButton::clicked, this, openSelectedUart);

    QPushButton *shutdownUart_exeButton = new QPushButton("CLOSE", this);
    shutdownUart_exeButton->setGeometry(w.xGap*3 + w.xText, w.yGap*17 + w.yLogo*4 + w.yUnit*10, w.xUnit*2, w.yUnit);

    auto shutdownSelectedUart = [this]()
    {
        shutdownUart();

    };
    connect(shutdownUart_exeButton, &QPushButton::clicked, this, shutdownSelectedUart);

    m_baudrate_dataField = new QLineEdit(this);
    m_baudrate_dataField->setGeometry(w.xGap, w.yGap*17 + w.yLogo*4 + w.yUnit*10, w.xText, w.yUnit);
    m_baudrate_dataField->setText("2000000");
    m_baudrate_dataField->setPlaceholderText("baudrate");

    QPushButton *clearConsole_exeButton = new QPushButton("CLEAN", this);
    clearConsole_exeButton->setGeometry(w.xGap*3 + w.xText, w.yGap*18 + w.yLogo*4 + w.yUnit*11, w.xUnit*2, w.yUnit);

    auto clearConsole = [this]()
    {
        clearUartConsole();

    };
    connect(clearConsole_exeButton, &QPushButton::clicked, this, clearConsole);

    QPushButton *refreshSerial_exeButton = new QPushButton("REFRESH", this);
    refreshSerial_exeButton->setGeometry(w.xGap, w.yGap*18 + w.yLogo*4 + w.yUnit*11, w.xText, w.yUnit);

    auto refreshSerial = [this]()
    {
        refreshSerialPorts();

    };
    connect(refreshSerial_exeButton, &QPushButton::clicked, this, refreshSerial);
}

void MainGui::setupUartConsole()
{
    /* Rx window */
    m_uartConsoleOutput = new QPlainTextEdit(this);
    m_uartConsoleOutput->setReadOnly(true);
    m_uartConsoleOutput->setGeometry(c.xPosition, w.yGap*16 + w.yLogo*3 + w.yUnit*9, c.xSize, w.yWindow - w.yGap*17 - w.yUnit*10 - w.yLogo*3);
    m_uartConsoleOutput->setPlainText("[INIT] UART Console Initialized");
    /* Tx bar */
    m_uartInput = new QLineEdit(this);
    m_uartInput->setGeometry(c.xPosition, w.yWindow - w.yGap - w.yUnit, c.xSize, w.yUnit);
    connect(m_uartInput, &QLineEdit::returnPressed, this, &MainGui::onUartInput);
}

void MainGui::clearUartConsole()
{
    if( m_uartConsoleOutput)
    {
        m_uartConsoleOutput->clear();
    }
}

void MainGui::refreshSerialPorts()
{
    // Clear the current list
    m_serialPortDropdown->clear();

    // Get available ports
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports)
    {
        m_serialPortDropdown->addItem(info.portName());
    }

    // Handle the case where no devices are found
    if (ports.isEmpty())
    {
        m_serialPortDropdown->addItem("No devices found");
        m_serialPortDropdown->setEnabled(false);
    }
    else
    {
        m_serialPortDropdown->setEnabled(true);
    }

    // Optional: print to console so you know it ran
    printToUartConsole("[INFO] Serial port list refreshed");
}

void MainGui::openUart()
{
    m_serialPort = new QSerialPort(this);

    m_uartPortName = m_serialPortDropdown->currentText();
    m_serialPort->setPortName(m_uartPortName);
#if 1
    bool ok = false;
    int baudRate = m_baudrate_dataField->text().toInt(&ok);

    if (ok)
    {
        m_serialPort->setBaudRate(baudRate);
    }
    else
    {
        printToUartConsole("[ERROR] Invalid baud rate entered.");
    }
#else
    m_serialPort->setBaudRate(115200);
#endif
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite))
    {
        m_uartIsConnected = true;
        printToUartConsole("[INIT] UART Connection established");
    }
    else
    {
        m_uartIsConnected = false;
        printToUartConsole("[INIT] UART Failed to open port");
    }

    connect(m_serialPort, &QSerialPort::readyRead, this, &MainGui::readUartData);
}

void MainGui::readUartData()
{
#if 0
    if (m_uartIsConnected && m_serialPort->isOpen())
    {
        m_readBuffer.append(m_serialPort->readAll());

        const int messageLength = 8;
        while (m_readBuffer.size() >= messageLength)
        {
            QByteArray completeMessage = m_readBuffer.left(messageLength);
            m_readBuffer.remove(0, messageLength);

            m_currentTime = QDateTime::currentDateTime().toString("HH:mm:ss");
            printToUartConsole("[" + m_currentTime + "] UART Rx: " + completeMessage);
        }
        // m_readBuffer.clear();
    }
#else
    if (m_uartIsConnected && m_serialPort->isOpen())
    {
        m_readBuffer.append(m_serialPort->readAll());

        const int messageLength = 8;
        while (m_readBuffer.size() >= messageLength)
        {
            QByteArray completeMessage = m_readBuffer.left(messageLength);
            m_readBuffer.remove(0, messageLength);

            printToUartConsole(completeMessage);
        }
        // m_readBuffer.clear();
    }
#endif
}

void MainGui::writeToUart(const QString &data)
{
#if 0
    m_currentTime = QDateTime::currentDateTime().toString("HH:mm:ss");
    if (m_uartIsConnected && m_serialPort->isOpen())
    {
        m_serialPort->write(data.toUtf8());
        printToUartConsole("[" + m_currentTime + "] UART Tx: " + data);
    }
    else
    {
        printToUartConsole("[" + m_currentTime + "] UART is not connected.");
    }
#else
    if (m_uartIsConnected && m_serialPort->isOpen())
    {
        m_serialPort->write(data.toUtf8());
        printToUartConsole(data);
    }
    else
    {
        printToUartConsole("UART is not connected");
    }
#endif
}

void MainGui::onUartInput()
{
    QString inputData = m_uartInput->text();
    if (!inputData.isEmpty())
    {
        writeToUart(inputData);
        m_uartInput->clear();
    }
}

void MainGui::shutdownUart()
{
    if (m_uartIsConnected && m_serialPort->isOpen())
    {
        m_serialPort->close();
        printToUartConsole("[EXIT] Connection closed");
    }
}

void MainGui::printToUartConsole(const QString &message)
{
    m_uartConsoleOutput->appendPlainText(message);
}

void MainGui::setupNetworkControl()
{
    QLabel *netLabel = new QLabel("NET.CTRL", this);
    QFont netLabelFont;
    netLabelFont.setPointSize(30);
    netLabelFont.setItalic(true);
    netLabelFont.setBold(true);
    netLabel->setFont(netLabelFont);
    netLabel->setGeometry(w.xGap*5 + w.xText + w.xUnit*2, w.yGap, w.xLogo, w.yLogo);

    QPushButton *icmpButton = new QPushButton("ICMP.SCAN", this);
    icmpButton->setGeometry(w.xGap*5 + w.xText + w.xUnit*2, w.yGap*2 + w.yLogo, w.xUnit*2, w.yUnit);
    icmpButton->setStyleSheet(
        "QPushButton {"
        "   background-color: blue;"
        "   color: white;"
        "   font-size: 17px;"
        "   font-weight: bold;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: darkblue;"
        "}"
        "QPushButton:pressed {"
        "   background-color: black;"
        "}"
    );

    auto icmpScan = [this]()
    {
        if (NULL == m_instanceKernelCtrl)
        {
            printToMainConsole("$ Main Control is Down");
            return;
        }
        else
        {
            printToMainConsole("$ Perform ICMP Network Scan");
            m_instanceKernelCtrl->sendCommand(CTRL_CMD_NET_ICMP);
        }
    };
    connect(icmpButton, &QPushButton::clicked, this, icmpScan);
}

void MainGui::setupMainCtrl()
{
    QLabel *mainCtrlLabel = new QLabel("MAIN.CTR", this);
    QFont mainCtrlLabelFont;
    mainCtrlLabelFont.setPointSize(30);
    mainCtrlLabelFont.setItalic(true);
    mainCtrlLabelFont.setBold(true);
    mainCtrlLabel->setFont(mainCtrlLabelFont);
    mainCtrlLabel->setGeometry(800 - w.xGap*2 - w.xUnit*4 , w.yGap, w.xLogo, w.yLogo);

    QPushButton *connectButton = new QPushButton("NEW", this);
    connectButton->setGeometry(800 - w.xGap*2 - w.xUnit*4, w.yGap*2 + w.yLogo, w.xUnit*2, w.yUnit);
    connectButton->setStyleSheet(
        "QPushButton {"
        "   background-color: green;"
        "   color: white;"
        "   font-size: 17px;"
        "   font-weight: bold;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: darkgreen;"
        "}"
        "QPushButton:pressed {"
        "   background-color: black;"
        "}"
    );
    connect(connectButton, &QPushButton::clicked, this, &MainGui::createMainCtrl);

    QPushButton *terminateButton = new QPushButton("DELETE", this);
    terminateButton->setGeometry(800 - w.xGap*2 - w.xUnit*4, w.yGap*3 + w.yUnit*3, w.xUnit*2, w.yUnit);
    terminateButton->setStyleSheet(
        "QPushButton {"
        "   background-color: red;"
        "   color: white;"
        "   font-size: 17px;"
        "   font-weight: bold;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: darkred;"
        "}"
        "QPushButton:pressed {"
        "   background-color: black;"
        "}"
    );
    connect(terminateButton, &QPushButton::clicked, this, &MainGui::deleteMainCtrl);
}

void MainGui::setupSeparators()
{
    /* Vertical Separator */
    QFrame *vLine1 = new QFrame(this);
    vLine1->setGeometry(w.xGap*4 + w.xText + w.xUnit*2, w.yGap, w.separatorWidth , w.yWindow - w.yGap*2);
    vLine1->setFrameShape(QFrame::VLine);
    vLine1->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine1 = new QFrame(this);
    hLine1->setGeometry(w.xGap, w.yGap*5 + w.yLogo + w.yUnit*3, w.xWindow - w.xGap*2, w.separatorWidth);
    hLine1->setFrameShape(QFrame::HLine);
    hLine1->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine2 = new QFrame(this);
    hLine2->setGeometry(w.xGap, w.yGap*10 + w.yLogo*2 + w.yUnit*6, w.xGap*3 + w.xText + w.xUnit*2, w.separatorWidth);
    hLine2->setFrameShape(QFrame::HLine);
    hLine2->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine3 = new QFrame(this);
    hLine3->setGeometry(800, w.yGap*10 + w.yLogo*2 + w.yUnit*6, w.xWindow - 800 - w.xGap, w.separatorWidth);
    hLine3->setFrameShape(QFrame::HLine);
    hLine3->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine4 = new QFrame(this);
    hLine4->setGeometry(w.xGap, w.yGap*15 + w.yLogo*3 + w.yUnit*9, w.xWindow - w.xGap*2, w.separatorWidth);
    hLine4->setFrameShape(QFrame::HLine);
    hLine4->setFrameShadow(QFrame::Sunken);
    /* Vertical Separator */
    QFrame *vLine2 = new QFrame(this);
    vLine2->setGeometry(800 - w.xGap*3 - w.xUnit*4 , w.yGap, w.separatorWidth, w.yGap*4 + w.yLogo + w.yUnit*3);
    vLine2->setFrameShape(QFrame::VLine);
    vLine2->setFrameShadow(QFrame::Sunken);
    /* Vertical Separator */
    QFrame *vLine3 = new QFrame(this);
    vLine3->setGeometry(800, w.yGap, w.separatorWidth, w.yGap*14 + w.yLogo*3 + w.yUnit*9);
    vLine3->setFrameShape(QFrame::VLine);
    vLine3->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine5 = new QFrame(this);
    hLine5->setGeometry(w.xGap, w.yGap*20 + w.yLogo*4 + w.yUnit*12, w.xGap*3 + w.xText + w.xUnit*2, w.separatorWidth);
    hLine5->setFrameShape(QFrame::HLine);
    hLine5->setFrameShadow(QFrame::Sunken);
}

void MainGui::createMainCtrl()
{
    if (!m_instanceKernelCtrl)
    {
        /* Create Kernel Control Instance */
        m_instanceKernelCtrl = std::make_unique<KernelCtrl>();
        /* Configure Shared pointers for modified Observert */
        m_instanceKernelCtrl->setTransferPointers(m_Rx_MainGuiVector, m_Tx_MainGuiVector, m_IO_MainGuiState);
        /* Trigger configuration Event */
        m_instanceKernelCtrl->setKernelCtrlState(KERNEL_CTRL_INIT);
    }
    else
    {
        std::cout << "[INFO] [CTR] Kernel Control -> Already Created" << std::endl;
    }
}

void MainGui::deleteMainCtrl()
{
    if(m_instanceKernelCtrl)
    {
        if(true == m_instanceKernelCtrl->getKernelConnected())
        {
            // m_instanceKernelCtrl->sendCommand(CMD_DEBUG_DISABLE);
        }

        m_instanceKernelCtrl.reset();
    }
    else
    {
        std::cout << "[INFO] [CTR] Kernel Control -> Already Destroyed" << std::endl;
    }
}
