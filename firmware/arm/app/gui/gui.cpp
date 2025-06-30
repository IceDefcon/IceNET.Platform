/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <iostream>
#include "gui.h"

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

/* GET */ const mainWindowType* gui::getMainWindow()
{
    return &w;
}

/* GET */ const mainConsoleType* gui::getMainConsole()
{
    return &c;
}

gui::gui() :
    m_Rx_GuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
    m_Tx_GuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
    m_IO_GuiState(std::make_shared<ioStateType>(IO_COM_IDLE)),
    m_isPulseControllerEnabled(true),
    m_isStartAcquisition(true),
    m_isDebugEnabled(true),
    m_CsiCamera(nullptr),
    m_UsbCamera(nullptr)
{
    qDebug() << "[MAIN] [CONSTRUCTOR]" << this << "::  gui";

    setupWindow();
    setupMainConsole();
    setupUartConsole();
    setupFpgaCtrl();
    setupDroneControl();
    setupI2C();
    setupSPI();
    setupPWM();
    setupDMA();
    setupCMD();
    setupEXT();
    setupSeparators();
    setupCameras();
    setupAdditionalDebugs();
}


gui::~gui()
{
    shutdownUart();
    deleteDroneControl();

    qDebug() << "[MAIN] [DESTRUCTOR]" << this << ":: gui ";
}

void gui::setupWindow()
{
    setWindowTitle("IceNET Platform");
    setFixedSize(w.xWindow, w.yWindow);
}

void gui::setupMainConsole()
{
    m_mainConsoleOutput = new QPlainTextEdit(this);
    m_mainConsoleOutput->setReadOnly(true);
    m_mainConsoleOutput->setGeometry(c.xPosition, c.yPosition, c.xSize - 400, c.ySize);
    m_mainConsoleOutput->setPlainText("[INIT] Main Console Initialized...");
}

void gui::setupUartConsole()
{
    /* Rx window */
    m_uartConsoleOutput = new QPlainTextEdit(this);
    m_uartConsoleOutput->setReadOnly(true);
    m_uartConsoleOutput->setGeometry(c.xPosition, w.yGap*16 + w.yLogo*3 + w.yUnit*9, c.xSize, w.yWindow - w.yGap*17 - w.yUnit*10 - w.yLogo*3);
    m_uartConsoleOutput->setPlainText("[INIT] UART Console Initialized");
    /* Tx bar */
    m_uartInput = new QLineEdit(this);
    m_uartInput->setGeometry(c.xPosition, w.yWindow - w.yGap - w.yUnit, c.xSize, w.yUnit);
    connect(m_uartInput, &QLineEdit::returnPressed, this, &gui::onUartInput);
    /* Open device */
    openUart();
}

void gui::setupFpgaCtrl()
{
    QLabel *reset_label = new QLabel("IRQ", this);
    QFont reset_labelFont;
    reset_labelFont.setPointSize(30);
    reset_labelFont.setItalic(true);
    reset_labelFont.setBold(true);
    reset_label->setFont(reset_labelFont);
    reset_label->setGeometry(w.xGap*5 + w.xText + w.xUnit*2, w.yGap, w.xLogo, w.yLogo);

    QPushButton *resetButton = new QPushButton("RESET", this);
    resetButton->setGeometry(w.xGap*5 + w.xText + w.xUnit*2, w.yGap*2 + w.yLogo, w.xUnit*2, w.yUnit);
    resetButton->setStyleSheet(
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

    auto fpgaReset = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            m_instanceDroneControl->sendCommand(CMD_FPGA_RESET);
        }
    };
    connect(resetButton, &QPushButton::clicked, this, fpgaReset);

    QPushButton *offloadButton = new QPushButton("OFF.CFG", this);
    offloadButton->setGeometry(w.xGap*5 + w.xText + w.xUnit*2, w.yGap*3 + w.yLogo + w.yUnit, w.xUnit*2, w.yUnit);
    offloadButton->setStyleSheet(
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

    auto primaryOffload = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ Primary Offload from the FIFO");
            interruptVector_execute(VECTOR_OFFLOAD_PRIMARY);
        }
    };
    connect(offloadButton, &QPushButton::clicked, this, primaryOffload);

    QPushButton *externalOffloadButton = new QPushButton("OFF.EXT", this);
    externalOffloadButton->setGeometry(w.xGap*6 + w.xText + w.xUnit*4, w.yGap*3 + w.yLogo + w.yUnit, w.xUnit*2, w.yUnit);
    externalOffloadButton->setStyleSheet(
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

    auto externalOffload = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ External Offload from the FIFO");
            interruptVector_execute(VECTOR_OFFLOAD_EXTERNAL);
        }
    };
    connect(externalOffloadButton, &QPushButton::clicked, this, externalOffload);

    m_enableButton = new QPushButton("ENABLE", this);
    m_enableButton->setGeometry(w.xGap*5 + w.xText + w.xUnit*2, w.yGap*4 + w.yLogo + w.yUnit*2, w.xUnit*2, w.yUnit);

    m_enableButton->setStyleSheet(
        "QPushButton {"
        "   background-color: green;"
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

    auto pulseController = [this]()
    {
        if (NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
            return;
        }

        if (m_isPulseControllerEnabled)
        {
            m_enableButton->setText("DISABLE");
            m_enableButton->setStyleSheet(
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

            printToMainConsole("$ Enable Pulse Controllers");
            interruptVector_execute(VECTOR_ENABLE);
        }
        else
        {
            m_enableButton->setText("ENABLE");
            m_enableButton->setStyleSheet(
                "QPushButton {"
                "   background-color: green;"
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

            printToMainConsole("$ Disable Pulse Controllers");
            interruptVector_execute(VECTOR_DISABLE);
        }

        m_isPulseControllerEnabled = !m_isPulseControllerEnabled;
    };
    connect(m_enableButton, &QPushButton::clicked, this, pulseController);

    m_startButton = new QPushButton("START", this);
    m_startButton->setGeometry(w.xGap*6 + w.xText + w.xUnit*4, w.yGap*4 + w.yLogo + w.yUnit*2, w.xUnit*2, w.yUnit);

    m_startButton->setStyleSheet(
        "QPushButton {"
        "   background-color: green;"
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

    auto acquisitionControl = [this]()
    {
        if (NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
            return;
        }

        if (m_isStartAcquisition)
        {
            m_startButton->setText("STOP");
            m_startButton->setStyleSheet(
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

            printToMainConsole("$ Start Secondary SPI/DMA");
            interruptVector_execute(VECTOR_START);
        }
        else
        {
            m_startButton->setText("START");
            m_startButton->setStyleSheet(
                "QPushButton {"
                "   background-color: green;"
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

            printToMainConsole("$ Stop Secondary SPI/DMA");
            interruptVector_execute(VECTOR_STOP);
        }

        m_isStartAcquisition = !m_isStartAcquisition;
    };
    connect(m_startButton, &QPushButton::clicked, this, acquisitionControl);
}

void gui::setupDroneControl()
{
    QLabel *thread_label = new QLabel("DRONE", this);
    QFont thread_labelFont;
    thread_labelFont.setPointSize(30);
    thread_labelFont.setItalic(true);
    thread_labelFont.setBold(true);
    thread_label->setFont(thread_labelFont);
    thread_label->setGeometry(800 - w.xGap*2 - w.xUnit*4 , w.yGap, w.xLogo, w.yLogo);

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
    connect(connectButton, &QPushButton::clicked, this, &gui::createDroneControl);

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
    connect(terminateButton, &QPushButton::clicked, this, &gui::deleteDroneControl);
}

void gui::setupSeparators()
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

void gui::setupCameras()
{
    QPushButton *usbCameraButton = new QPushButton("USB.CAM", this);
    usbCameraButton->setGeometry(800 - w.xGap*1 - w.xUnit*2, w.yGap*2 + w.yLogo, w.xUnit*2, w.yUnit);
    usbCameraButton->setStyleSheet(
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
    connect(usbCameraButton, &QPushButton::clicked, this, &gui::openUsbCamera);

    QPushButton *csiCameraButton = new QPushButton("CSI.CAM", this);
    csiCameraButton->setGeometry(800 - w.xGap*1 - w.xUnit*2, w.yGap*3 + w.yLogo + w.yUnit, w.xUnit*2, w.yUnit);
    csiCameraButton->setStyleSheet(
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
    connect(csiCameraButton, &QPushButton::clicked, this, &gui::openCsiCamera);
}

void gui::setupAdditionalDebugs()
{
    QPushButton *f0Button = new QPushButton("TRIGGER", this);
    f0Button->setGeometry(w.xGap*6 + w.xText + w.xUnit*4, w.yGap*2 + w.yLogo, w.xUnit*2, w.yUnit);
    f0Button->setStyleSheet(
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

    auto executeF0 = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ External SPI Trigger Vector");
            interruptVector_execute(VECTOR_TRIGGER);
        }
    };
    connect(f0Button, &QPushButton::clicked, this, executeF0);

    QPushButton *f1Button = new QPushButton("F1", this);
    f1Button->setGeometry(w.xGap*7 + w.xText + w.xUnit*6, w.yGap*2 + w.yLogo, w.xUnit*2, w.yUnit);
    f1Button->setStyleSheet(
        "QPushButton {"
        "   background-color: purple;"
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

    auto executeF1 = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ F1 Test Vector");
            interruptVector_execute(VECTOR_F1);
        }
    };
    connect(f1Button, &QPushButton::clicked, this, executeF1);

    QPushButton *f2Button = new QPushButton("F2", this);
    f2Button->setGeometry(w.xGap*7 + w.xText + w.xUnit*6, w.yGap*3 + w.yLogo + w.yUnit, w.xUnit*2, w.yUnit);
    f2Button->setStyleSheet(
        "QPushButton {"
        "   background-color: purple;"
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

    auto executeF2 = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ F2 Test Vector");
            interruptVector_execute(VECTOR_F2);
        }
    };
    connect(f2Button, &QPushButton::clicked, this, executeF2);

    QPushButton *f3Button = new QPushButton("F3", this);
    f3Button->setGeometry(w.xGap*7 + w.xText + w.xUnit*6, w.yGap*4 + w.yLogo + w.yUnit*2, w.xUnit*2, w.yUnit);
    f3Button->setStyleSheet(
        "QPushButton {"
        "   background-color: purple;"
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

    auto executeF3 = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ F3 Test Vector");
            interruptVector_execute(VECTOR_F3);
        }
    };
    connect(f3Button, &QPushButton::clicked, this, executeF3);

    /* Kernel Debug */
    m_debugButton = new QPushButton("DBG.ON", this);
    m_debugButton->setGeometry(800 - w.xGap*1 - w.xUnit*2, w.yGap*4 + w.yLogo + w.yUnit*2, w.xUnit*2, w.yUnit);

    m_debugButton->setStyleSheet(
        "QPushButton {"
        "   background-color: green;"
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

    auto debugControl = [this]()
    {
        if (NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
            return;
        }

        if (m_isDebugEnabled)
        {
            m_debugButton->setText("DBG.OFF");
            m_debugButton->setStyleSheet(
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

            printToMainConsole("$ Enable Kernel Debug");
            m_instanceDroneControl->sendCommand(CMD_DEBUG_ENABLE);
        }
        else
        {
            m_debugButton->setText("DBG.ON");
            m_debugButton->setStyleSheet(
                "QPushButton {"
                "   background-color: green;"
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

            printToMainConsole("$ Disable Kernel Debug");
            m_instanceDroneControl->sendCommand(CMD_DEBUG_DISABLE);
        }

        m_isDebugEnabled = !m_isDebugEnabled;
    };
    connect(m_debugButton, &QPushButton::clicked, this, debugControl);
}

void gui::setupDMA()
{
    /* DMA :: Row[0] */
    QLabel *dma_label = new QLabel("DMA", this);
    QFont dma_labelFont;
    dma_labelFont.setPointSize(30);
    dma_labelFont.setItalic(true);
    dma_labelFont.setBold(true);
    dma_label->setFont(dma_labelFont);
    dma_label->setGeometry(w.xGap, w.yGap*16 + w.yLogo*3 + w.yUnit*9, w.xLogo, w.yLogo);
    /* DMA :: Row[1] */
    QLabel *dma_customLabel = new QLabel("[FPGA->CPU] Custom", this);
    dma_customLabel->setGeometry(w.xGap, w.yGap*16 + w.yLogo*4 + w.yUnit*9, w.xText, w.yUnit);
    m_dmaCustom_dataField = new QLineEdit(this);
    m_dmaCustom_dataField->setGeometry(w.xGap*2 + w.xText, w.yGap*16 + w.yLogo*4 + w.yUnit*9, w.xUnit, w.yUnit);
    m_dmaCustom_dataField->setText("0x01");
    QPushButton *dma_custom_exeButton = new QPushButton("EXE", this);
    dma_custom_exeButton->setGeometry(w.xGap*3 + w.xText + w.xUnit, w.yGap*16 + w.yLogo*4 + w.yUnit*9, w.xUnit, w.yUnit);

    auto dmaCustom = [this]()
    {
        dma_execute(CMD_DMA_CUSTOM);

    };
    connect(dma_custom_exeButton, &QPushButton::clicked, this, dmaCustom);

    /* DMA :: Row[2] */
    QLabel *dma_singleLabel = new QLabel("[FPGA->CPU] Single", this);
    dma_singleLabel->setGeometry(w.xGap, w.yGap*17 + w.yLogo*4 + w.yUnit*10, w.xText, w.yUnit);
    QPushButton *dma_single_exeButton = new QPushButton("EXE", this);
    dma_single_exeButton->setGeometry(w.xGap*3 + w.xText + w.xUnit, w.yGap*17 + w.yLogo*4 + w.yUnit*10, w.xUnit, w.yUnit);

    auto dmaSingle = [this]()
    {
        dma_execute(CMD_DMA_SINGLE);

    };
    connect(dma_single_exeButton, &QPushButton::clicked, this, dmaSingle);

    /* DMA :: Row[3] */
    QLabel *i2c_registerLabel = new QLabel("[FPGA->CPU] Sensor", this);
    i2c_registerLabel->setGeometry(w.xGap, w.yGap*18 + w.yLogo*4 + w.yUnit*11, w.xText, w.yUnit);
    QPushButton *dmaSensor_exeButton = new QPushButton("EXE", this);
    dmaSensor_exeButton->setGeometry(w.xGap*3 + w.xText + w.xUnit, w.yGap*18 + w.yLogo*4 + w.yUnit*11, w.xUnit, w.yUnit);

    auto dmaSensor = [this]()
    {
        dma_execute(CMD_DMA_SENSOR);

    };
    connect(dmaSensor_exeButton, &QPushButton::clicked, this, dmaSensor);
}

void gui::setupCMD()
{
    /* FIFO :: Row[0] */
    QLabel *cmd_label = new QLabel("CMD", this);
    QFont cmd_labelFont;
    cmd_labelFont.setPointSize(30);
    cmd_labelFont.setItalic(true);
    cmd_labelFont.setBold(true);
    cmd_label->setFont(cmd_labelFont);
    cmd_label->setGeometry(800 + w.xGap, w.yGap, w.xLogo, w.yLogo);

    QPushButton *readOnlyButton = new QPushButton("READ", this);
    readOnlyButton->setGeometry(800 + w.xGap, w.yGap*2 + w.yLogo, w.xUnit*2, w.yUnit);
    readOnlyButton->setStyleSheet(
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

    auto readOnlyMode = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ Set Commander State Machine -> IO_COM_READ_ONLY");
            m_instanceDroneControl->setCommanderState(IO_COM_READ_ONLY);
        }
    };
    connect(readOnlyButton, &QPushButton::clicked, this, readOnlyMode);

    QPushButton *idleButton = new QPushButton("IDLE", this);
    idleButton->setGeometry(800 + w.xGap, w.yGap*3 + w.yUnit*3, w.xUnit*2, w.yUnit);
    idleButton->setStyleSheet(
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

    auto idleMode = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ Set Commander State Machine -> IO_COM_IDLE");
            m_instanceDroneControl->setCommanderState(IO_COM_IDLE);
        }
    };
    connect(idleButton, &QPushButton::clicked, this, idleMode);

    QPushButton *getCalibButton = new QPushButton("GET.CLB", this);
    getCalibButton->setGeometry(800 + w.xGap, w.yGap*4 + w.yUnit*4, w.xUnit*2, w.yUnit);
    getCalibButton->setStyleSheet(
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

    auto getCalibrationMode = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ Set Commander State Machine -> IO_COM_GET_CALIBRATION");
            m_instanceDroneControl->setCommanderState(IO_COM_GET_CALIBRATION);
        }
    };
    connect(getCalibButton, &QPushButton::clicked, this, getCalibrationMode);

    QPushButton *setCalibButton = new QPushButton("SET.CLB", this);
    setCalibButton->setGeometry(800 + w.xGap*2 + w.xUnit*2, w.yGap*4 + w.yUnit*4, w.xUnit*2, w.yUnit);
    setCalibButton->setStyleSheet(
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

    auto setCalibrationMode = [this]()
    {
        if(NULL == m_instanceDroneControl)
        {
            printToMainConsole("$ Drone Control is Down");
        }
        else
        {
            printToMainConsole("$ Set Commander State Machine -> IO_COM_SET_CALIBRATION");
            m_instanceDroneControl->setCommanderState(IO_COM_SET_CALIBRATION);
        }
    };
    connect(setCalibButton, &QPushButton::clicked, this, setCalibrationMode);
}

void gui::setDeadCommand()
{
    /* 0xDEAD Code :: In case if something happen */
    (*m_Tx_GuiVector)[0] = 0xDE;
    (*m_Tx_GuiVector)[1] = 0xAD;
    (*m_Tx_GuiVector)[2] = 0xC0;
    (*m_Tx_GuiVector)[3] = 0xDE;
    (*m_Tx_GuiVector)[4] = 0x22;
    (*m_Tx_GuiVector)[5] = 0x22;
    (*m_Tx_GuiVector)[6] = 0x22;
    (*m_Tx_GuiVector)[7] = 0x22;
}

void gui::setDummyCommand()
{
    /* 0x7E Code :: In case if something happen */
    (*m_Tx_GuiVector)[0] = 0x7E;
    (*m_Tx_GuiVector)[1] = 0x7E;
    (*m_Tx_GuiVector)[2] = 0x7E;
    (*m_Tx_GuiVector)[3] = 0x7E;
    (*m_Tx_GuiVector)[4] = 0x44;
    (*m_Tx_GuiVector)[5] = 0x44;
    (*m_Tx_GuiVector)[6] = 0x44;
    (*m_Tx_GuiVector)[7] = 0x44;
}

std::string gui::vectorToString(interruptVectorType type)
{
    switch (type)
    {
        case VECTOR_RESERVED:           return "VECTOR_RESERVED";
        case VECTOR_OFFLOAD_PRIMARY:    return "VECTOR_OFFLOAD_PRIMARY";    /* FIFO Offload chain */
        case VECTOR_ENABLE:             return "VECTOR_ENABLE";             /* Enable Pulse Controllers */
        case VECTOR_DISABLE:            return "VECTOR_DISABLE";            /* Disable Pulse Controllers */
        case VECTOR_START:              return "VECTOR_START";              /* Start Measurement Acquisition */
        case VECTOR_STOP:               return "VECTOR_STOP";               /* Stop Measurement Acquisition */
        case VECTOR_OFFLOAD_EXTERNAL:   return "VECTOR_OFFLOAD_EXTERNAL";   /* FIFO Offload External chain */
        case VECTOR_TRIGGER:            return "VECTOR_TRIGGER";            /* Trigger Vector */
        case VECTOR_F1:                 return "VECTOR_F1";                 /* Debug Test Vector */
        case VECTOR_F2:                 return "VECTOR_F2";                 /* Debug Test Vector */
        case VECTOR_F3:                 return "VECTOR_F3";                 /* Debug Test Vector */
        case VECTOR_UNUSED_11:          return "VECTOR_UNUSED_11";               
        case VECTOR_UNUSED_12:          return "VECTOR_UNUSED_12";
        case VECTOR_UNUSED_13:          return "VECTOR_UNUSED_13";
        case VECTOR_UNUSED_14:          return "VECTOR_UNUSED_14";
        case VECTOR_UNUSED_15:          return "VECTOR_UNUSED_15";
        default:                        return "UNKNOWN_VECTOR";
    }
}
/**
 *
 * TODO
 *
 * This need to be parametrized
 *
 */
void gui::setInterruptVector(uint8_t vector)
{
    //////////////////////////////////////////////////////////////////////////////
    // Vector Table
    //////////////////////////////////////////////////////////////////////////////
    //
    //  OFFLOAD_CTRL :: 8-bits
    //
    //  Dma config (Auto/Manual Config)
    //      |
    //      |        Device (I2C, SPI, PWM, INT)
    //      |          ID
    //      |  Vector  ||
    //      |   ||||   ||
    //      V   VVVV   VV
    //    | x | xxxx | xx | x | << OFFLOAD_CTRL : std_logic_vector(7 downto 0)
    //          ΛΛΛΛ        Λ
    //          ||||        |
    //          ||||        |
    //          ||||        |
    //       burst size    R/W (I2C, SPI)
    //       (I2C, SPI)
    //
    //////////////////////////////////////////////////////////////////////////////
    (*m_Tx_GuiVector)[0] = vector;
    (*m_Tx_GuiVector)[1] = 0xAF;
    (*m_Tx_GuiVector)[2] = 0xAE;
    (*m_Tx_GuiVector)[3] = 0xAD;
    (*m_Tx_GuiVector)[4] = 0x00;
    (*m_Tx_GuiVector)[5] = 0x00;
    (*m_Tx_GuiVector)[6] = 0x00;
    (*m_Tx_GuiVector)[7] = 0x00;
}

void gui::interruptVector_execute(interruptVectorType type)
{
    //////////////////////////////////////////////////////////////////////////////
    //
    //  OFFLOAD_CTRL :: Vector Base :: 0x86
    //
    //  Dma config (Auto/Manual Config)
    //      |
    //      |        Device (I2C, SPI, PWM, INT)
    //      |          ID
    //      |  Vector  ||
    //      |   ||||   ||
    //      V   VVVV   VV
    //    | 1 | size | 11 | 0 | << OFFLOAD_CTRL : std_logic_vector(7 downto 0)
    //
    //////////////////////////////////////////////////////////////////////////////
    uint8_t intVector = 0x86; /* Base :: RESERVED Vector */
    intVector += ((uint8_t)type << 3);

    setInterruptVector(intVector);
    std::cout << "[INFO] [INT] Set Interrupt Vector -> " << vectorToString(type) << std::endl;

    m_instanceDroneControl->setCommanderState(IO_COM_WRITE_ONLY);
    printToMainConsole("$ Set Interrupt Vector -> " + QString::fromStdString(vectorToString(type)));
}

std::string gui::cmdToString(commandType cmd)
{
    switch (cmd)
    {
        case CMD_DMA_NORMAL:    return "CMD_DMA_NORMAL";
        case CMD_DMA_SENSOR:    return "CMD_DMA_SENSOR";
        case CMD_DMA_SINGLE:    return "CMD_DMA_SINGLE";
        case CMD_DMA_CUSTOM:    return "CMD_DMA_CUSTOM";
        case CMD_RAMDISK_CONFIG:return "CMD_RAMDISK_CONFIG";
        case CMD_RAMDISK_CLEAR: return "CMD_RAMDISK_CLEAR";
        case CMD_DEBUG_ENABLE:  return "CMD_DEBUG_ENABLE";
        case CMD_DEBUG_DISABLE: return "CMD_DEBUG_DISABLE";
        case CMD_FPGA_RESET:    return "CMD_FPGA_RESET";
        default:                return "UNKNOWN_CMD";
    }
}

void gui::dma_execute(commandType cmd)
{
    if(NULL == m_instanceDroneControl)
    {
        printToMainConsole("$ Drone Control is Down");
    }
    else
    {
        printToMainConsole("$ Send DMA Command to Kernel -> " + QString::fromStdString(cmdToString(cmd)));
        if(CMD_DMA_CUSTOM == cmd)
        {
            bool ok;
            QString dataText = m_dmaCustom_dataField->text();
            uint8_t dmaSize = static_cast<uint8_t>(dataText.toUInt(&ok, 16));

            m_instanceDroneControl->setDmaCustom(dmaSize);
            m_instanceDroneControl->sendCommand(cmd);
        }
        else if(CMD_DMA_SINGLE == cmd || CMD_DMA_SENSOR == cmd)
        {
            m_instanceDroneControl->sendCommand(cmd);
        }
        else
        {
            printToMainConsole("$ Wrong DMA Command");
        }
    }
}

void gui::i2c_execute()
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(NULL == m_instanceDroneControl)
    {
        printToMainConsole("$ Drone Control is Down");
    }
    else if(false == m_instanceDroneControl->getKernelConnected())
    {
        printToMainConsole("$ Kernel Communication is Down");
    }
    else
    {
        /* Dummy Command :: In case if something happen */
        setDummyCommand();

        int addressTemp, registerTemp, dataTemp;
        bool addressFlag, registerFlag,dataFlag;
        uint8_t addressValue, registerValue;

        uint8_t headerValue = 0x80; /* I2C Header Type */
        uint8_t dataValue = 0x00;

        QString addressText = m_i2c_addressField->text();
        QString registerText = m_i2c_registerField->text();

        addressTemp = addressText.toInt(&addressFlag, 0);
        registerTemp = registerText.toInt(&registerFlag, 0);

        if(addressTemp > 127 || registerTemp > 255)
        {
            if(addressTemp > 127) QMessageBox::warning(this, "Invalid Address", "Please enter a 7-bit Value");
            if(registerTemp > 255) QMessageBox::warning(this, "Invalid Register", "Please enter a 8-bit Value");
            return;
        }
        else
        {
            addressValue = static_cast<uint8_t>(addressTemp);
            registerValue = static_cast<uint8_t>(registerTemp);
        }

        if (m_i2c_writeTick->isChecked())
        {
            QString dataText = m_i2c_dataField->text();
            dataTemp = dataText.toInt(&dataFlag, 0);
            if(dataTemp > 255)
            {
                QMessageBox::warning(this, "Invalid Data", "Please enter a 8-bit Value");
                return;
            }
            else
            {
                headerValue += 0x01;
                dataValue = static_cast<uint8_t>(dataTemp);
            }
        }


        (*m_Tx_GuiVector)[0] = headerValue;
        (*m_Tx_GuiVector)[1] = addressValue;
        (*m_Tx_GuiVector)[2] = registerValue;
        (*m_Tx_GuiVector)[3] = dataValue;
        (*m_Tx_GuiVector)[4] = 0x00;
        (*m_Tx_GuiVector)[5] = 0x00;
        (*m_Tx_GuiVector)[6] = 0x00;
        (*m_Tx_GuiVector)[7] = 0x00;
        m_instanceDroneControl->setCommanderState(IO_COM_WRITE_ONLY);

        /* Wait for Kerenl to send data to FPGA */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printToMainConsole("$ Done");
    }
}

//////////////////////////////////////////////////////////////////////////////
//
//  OFFLOAD_CTRL :: 8-bits
//
//  Dma config (Auto/Manual Config)
//      |
//      |        Device (I2C, SPI, PWM)
//      |          ID
//      |          ||
//      |          ||
//      V          VV
//    | x | xxxx | xx | x | << OFFLOAD_CTRL : std_logic_vector(7 downto 0)
//          ΛΛΛΛ        Λ
//          ||||        |
//          ||||        |
//          ||||        |
//       burst size    R/W (I2C, SPI)
//       (I2C, SPI)
//
//////////////////////////////////////////////////////////////////////////////
void gui::spi_execute()
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(NULL == m_instanceDroneControl)
    {
        printToMainConsole("$ Drone Control is Down");
    }
    else if(false == m_instanceDroneControl->getKernelConnected())
    {
        printToMainConsole("$ Kernel Communication is Down");
    }
    else
    {
        /* Dummy Command :: In case if something happen */
        setDummyCommand();

        int addressTemp, registerTemp, dataTemp, burstTemp;
        bool addressFlag, registerFlag, dataFlag, burstFlag;
        uint8_t addressValue, registerValue, burstValue;

        uint8_t headerValue = 0x82; /* SPI Header Type */
        uint8_t dataValue = 0x00;

        QString addressText = m_spi_addressField->text();
        QString registerText = m_spi_registerField->text();
        QString burstText = m_spi_burstField->text();

        addressTemp = addressText.toInt(&addressFlag, 0);
        registerTemp = registerText.toInt(&registerFlag, 0);
        burstTemp = burstText.toInt(&burstFlag, 0);

        if((addressTemp < 17 || addressTemp > 20) || registerTemp > 255)
        {
            if(addressTemp > 127) QMessageBox::warning(this, "Invalid Address", "Please Range between 0x11 and 0x14");
            if(registerTemp > 255) QMessageBox::warning(this, "Invalid Register", "Please enter a 8-bit Value");
            return;
        }
        else
        {
            addressValue = static_cast<uint8_t>(addressTemp);
            registerValue = static_cast<uint8_t>(registerTemp);
        }

        if (m_spi_writeTick->isChecked())
        {
            QString dataText = m_spi_dataField->text();
            dataTemp = dataText.toInt(&dataFlag, 0);
            if(dataTemp > 255)
            {
                QMessageBox::warning(this, "Invalid Data", "Please enter a 8-bit Value");
                return;
            }
            else if(burstTemp > 15)
            {
                QMessageBox::warning(this, "Invalid Burst Size", "Burst lergth up to 15 supported");
            }
            else
            {
                headerValue += 0x01;
                dataValue = static_cast<uint8_t>(dataTemp);
            }
        }

        burstValue = static_cast<uint8_t>(burstTemp);
        burstValue <<= 3;
        headerValue += burstValue;

        (*m_Tx_GuiVector)[0] = headerValue;
        (*m_Tx_GuiVector)[1] = addressValue;
        (*m_Tx_GuiVector)[2] = registerValue;
        (*m_Tx_GuiVector)[3] = dataValue;
        (*m_Tx_GuiVector)[4] = 0x00;
        (*m_Tx_GuiVector)[5] = 0x00;
        (*m_Tx_GuiVector)[6] = 0x00;
        (*m_Tx_GuiVector)[7] = 0x00;
        m_instanceDroneControl->setCommanderState(IO_COM_WRITE_ONLY);

        /* Wait for Kerenl to send data to FPGA */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printToMainConsole("$ Done");
    }
}

void gui::ext_execute()
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(NULL == m_instanceDroneControl)
    {
        printToMainConsole("$ Drone Control is Down");
    }
    else if(false == m_instanceDroneControl->getKernelConnected())
    {
        printToMainConsole("$ Kernel Communication is Down");
    }
    else
    {
        /* Dummy Command :: In case if something happen */
        setDummyCommand();

        int byte0Temp, byte1Temp, byte2Temp;
        bool byte0Flag, byte1Flag, byte2Flag;
        uint8_t byte0Value, byte1Value, byte2Value, burstValue;

        uint8_t headerValue = 0x82; /* SPI Header Type */

        QString byte0Text = m_ext_Byte_0->text();
        QString byte1Text = m_ext_Byte_1->text();
        QString byte2Text = m_ext_Byte_2->text();

        byte0Temp = byte0Text.toInt(&byte0Flag, 0);
        byte1Temp = byte1Text.toInt(&byte1Flag, 0);
        byte2Temp = byte2Text.toInt(&byte2Flag, 0);

        if(byte0Temp > 255 || byte1Temp > 255 || byte2Temp > 255)
        {
            if(byte0Temp > 255) QMessageBox::warning(this, "Invalid Byte 0 Value", "Please enter a 8-bit Value");
            if(byte1Temp > 255) QMessageBox::warning(this, "Invalid Byte 0 Value", "Please enter a 8-bit Value");
            if(byte2Temp > 255) QMessageBox::warning(this, "Invalid Byte 0 Value", "Please enter a 8-bit Value");
            return;
        }
        else
        {
            byte0Value = static_cast<uint8_t>(byte0Temp);
            byte1Value = static_cast<uint8_t>(byte1Temp);
            byte2Value = static_cast<uint8_t>(byte2Temp);
        }

        if (m_ext_writeTick->isChecked())
        {
            headerValue += 0x01;
        }

        /**
         *
         * This chunk cause
         * header to be 0x92
         *
         */
        burstValue = static_cast<uint8_t>(0x02);
        burstValue <<= 3;
        headerValue += burstValue;

        (*m_Tx_GuiVector)[0] = headerValue;
        (*m_Tx_GuiVector)[1] = byte0Value;
        (*m_Tx_GuiVector)[2] = byte1Value;
        (*m_Tx_GuiVector)[3] = byte2Value;
        (*m_Tx_GuiVector)[4] = 0x00;
        (*m_Tx_GuiVector)[5] = 0x00;
        (*m_Tx_GuiVector)[6] = 0x00;
        (*m_Tx_GuiVector)[7] = 0x00;
        m_instanceDroneControl->setCommanderState(IO_COM_WRITE_ONLY);

        /* Wait for Kerenl to send data to FPGA */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printToMainConsole("$ Done");
    }
}

void gui::pwm_execute(pwmType type)
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(NULL == m_instanceDroneControl)
    {
        printToMainConsole("$ Drone Control is Down");
    }
    else if(false == m_instanceDroneControl->getKernelConnected())
    {
        printToMainConsole("$ Kernel Communication is Down");
    }
    else
    {
        /* Dummy Command :: In case if something happen */
        setDummyCommand();

        uint8_t headerValue = 0x84; /* PWM Header Type */
        uint8_t dataValue = 0x00;

        int dataTemp;
        bool dataFlag;
        QString dataText;
        dataText = m_pwm_dataField->text();
        dataTemp = dataText.toInt(&dataFlag, 0);

        if(dataTemp > 255)
        {
            QMessageBox::warning(this, "Invalid Data", "Please enter a 8-bit Value");
            return;
        }
        else
        {
            switch(type)
            {
                case PWM_EXE:
                    dataValue = static_cast<uint8_t>(dataTemp);
                    printToMainConsole("$ Execute");
                    break;

                case PWM_UP:
                    dataValue = static_cast<uint8_t>(dataTemp);
                    if(dataValue >= 0xF5)
                    {
                        dataValue = 0xFA;
                    }
                    else
                    {
                        dataValue += 0x05;
                    }
                    m_pwm_dataField->setText(QString("0x%1").arg(dataValue, 2, 16, QChar('0')).toLower());
                    printToMainConsole("$ Up");
                    break;

                case PWM_DOWN:
                    dataValue = static_cast<uint8_t>(dataTemp);
                    if(dataValue <= 0x05)
                    {
                        dataValue = 0x00;
                    }
                    else
                    {
                        dataValue -= 0x05;
                    }
                    m_pwm_dataField->setText(QString("0x%1").arg(dataValue, 2, 16, QChar('0')).toLower());
                    printToMainConsole("$ Down");
                    break;

                default:
                    printToMainConsole("$ Unknown type of operation");
                    break;
            };
        }

        (*m_Tx_GuiVector)[0] = headerValue;
        (*m_Tx_GuiVector)[1] = 0x00;
        (*m_Tx_GuiVector)[2] = 0x00;
        (*m_Tx_GuiVector)[3] = dataValue;
        (*m_Tx_GuiVector)[4] = 0x00;
        (*m_Tx_GuiVector)[5] = 0x00;
        (*m_Tx_GuiVector)[6] = 0x00;
        (*m_Tx_GuiVector)[7] = 0x00;
        m_instanceDroneControl->setCommanderState(IO_COM_WRITE_ONLY);

        /* Wait for Kerenl to send data to FPGA */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printToMainConsole("$ Done");
    }
}

void gui::openUart()
{
    m_serialPort = new QSerialPort(this);

    m_uartPortName = "/dev/ttyTHS1";
    m_serialPort->setPortName(m_uartPortName);
#if 1
    m_serialPort->setBaudRate(2000000);
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

    connect(m_serialPort, &QSerialPort::readyRead, this, &gui::readUartData);
}

void gui::readUartData()
{
    if (m_uartIsConnected && m_serialPort->isOpen())
    {
        m_readBuffer.append(m_serialPort->readAll());

#if 1
        const int messageLength = 8;
        while (m_readBuffer.size() >= messageLength)
        {
            QByteArray completeMessage = m_readBuffer.left(messageLength);
            m_readBuffer.remove(0, messageLength);

            m_currentTime = QDateTime::currentDateTime().toString("HH:mm:ss");

            printToUartConsole("[" + m_currentTime + "] UART Rx: " + completeMessage);
        }
#else
        const QByteArray lineEnding = "\r\n";
        while (m_readBuffer.contains(lineEnding))
        {
            int lineEndIndex = m_readBuffer.indexOf(lineEnding);
            QByteArray completeMessage = m_readBuffer.left(lineEndIndex);

            // Remove the processed message from the buffer
            m_readBuffer.remove(0, lineEndIndex + lineEnding.size());

            m_currentTime = QDateTime::currentDateTime().toString("HH:mm:ss");

            // Print the message without the line ending characters
            printToUartConsole("[" + m_currentTime + "] UART Rx: " + completeMessage);
        }
#endif
    }
}

void gui::writeToUart(const QString &data)
{
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
}

void gui::onUartInput()
{
    QString inputData = m_uartInput->text();
    if (!inputData.isEmpty())
    {
        writeToUart(inputData);
        m_uartInput->clear();
    }
}

void gui::shutdownUart()
{
    if (m_uartIsConnected && m_serialPort->isOpen())
    {
        m_serialPort->close();
        printToUartConsole("[EXIT] Connection closed");
    }
}

void gui::printToMainConsole(const QString &message)
{
    m_mainConsoleOutput->appendPlainText(message);
}

void gui::printToUartConsole(const QString &message)
{
    m_uartConsoleOutput->appendPlainText(message);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Modified RAII
//
// If we put this into the constructor
// and destructor we will have modified
// Resource Acquisition Is Initialisation
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gui::createDroneControl()
{
    if (!m_instanceDroneControl)
    {
        /* Create Drone Control Instance */
        m_instanceDroneControl = std::make_unique<DroneCtrl>();
        /* Configure Shared pointers for modified Observert */
        m_instanceDroneControl->setTransferPointers(m_Rx_GuiVector, m_Tx_GuiVector, m_IO_GuiState);
        /* Trigger configuration Event */
        m_instanceDroneControl->setDroneCtrlState(DRONE_CTRL_INIT);
    }
    else
    {
        std::cout << "[INFO] [GUI] Drone Control -> Already Created" << std::endl;
    }
}

void gui::deleteDroneControl()
{
    if(m_instanceDroneControl)
    {
        if(true == m_instanceDroneControl->getKernelConnected())
        {
            interruptVector_execute(VECTOR_DISABLE);
            interruptVector_execute(VECTOR_STOP);
            m_instanceDroneControl->sendCommand(CMD_DEBUG_DISABLE);
        }

        m_isPulseControllerEnabled = true;
        m_enableButton->setText("ENABLE");
        m_enableButton->setStyleSheet(
            "QPushButton {"
            "   background-color: green;"
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

        m_isStartAcquisition = true;
        m_startButton->setText("START");
        m_startButton->setStyleSheet(
            "QPushButton {"
            "   background-color: green;"
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

        m_isDebugEnabled = true;
        m_debugButton->setText("DBG.ON");
        m_debugButton->setStyleSheet(
            "QPushButton {"
            "   background-color: green;"
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

        m_instanceDroneControl.reset();
    }
    else
    {
        std::cout << "[INFO] [GUI] Drone Control -> Already Destroyed" << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void gui::C1_Execute()
{
    std::cout << "[INFO] [DEBUG] C1_Execute" << std::endl;
    //
    // TODO
    //
}

void gui::C2_Execute()
{
    std::cout << "[INFO] [DEBUG] C2_Execute" << std::endl;
    //
    // TODO
    //
}

void gui::C3_Execute()
{
    std::cout << "[INFO] [DEBUG] C3_Execute" << std::endl;
    //
    // TODO
    //
}

void gui::C4_Execute()
{
    std::cout << "[INFO] [DEBUG] C4_Execute" << std::endl;
    //
    // TODO
    //
}

void gui::openCsiCamera()
{
    if (!m_CsiCamera)
    {
        m_CsiCamera = new CsiCamera();
        m_CsiCamera->setAttribute(Qt::WA_DeleteOnClose);
        m_CsiCamera->show();

        auto CsiCamera = [this]()
        {
            m_CsiCamera = nullptr;
        };

        connect(m_CsiCamera, &QObject::destroyed, this, CsiCamera);
    }
    else
    {
        m_CsiCamera->raise();
        m_CsiCamera->activateWindow();
    }
}

void gui::openUsbCamera()
{
    if (!m_UsbCamera)
    {
        m_UsbCamera = new UsbCamera();
        m_UsbCamera->setAttribute(Qt::WA_DeleteOnClose);
        m_UsbCamera->show();

        auto UsbCamera = [this]()
        {
            m_UsbCamera = nullptr;
        };

        connect(m_UsbCamera, &QObject::destroyed, this, UsbCamera);
    }
    else
    {
        m_UsbCamera->raise();
        m_UsbCamera->activateWindow();
    }
}

