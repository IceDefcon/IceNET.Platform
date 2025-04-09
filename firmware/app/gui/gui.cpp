/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <iostream>
#include "gui.h"

static const deviceType dev =
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

static const consoleType console =
{
    .xPosition = dev.xGap*5 + dev.xText + dev.xUnit*2,  // Vertical Separator
    .yPosition = dev.yGap*6 + dev.yLogo + dev.yUnit*3,  // At SPI Logo
    .xSize = dev.xWindow - console.xPosition - dev.xGap,        // Obvious
    .ySize = dev.yGap*8 + dev.yLogo*2 + dev.yUnit*6+1,  // Last Horizontal Separator - yPosition + yGap
};

gui::gui() :
m_threadKill(true),
m_isKernelConnected(false),
m_Rx_GuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_Tx_GuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_IO_GuiState(std::make_shared<ioStateType>(IO_IDLE)),
m_isPulseControllerEnabled(true),
m_isStartAcquisition(true),
m_phase(0.0)
{
    qDebug() << "[MAIN] [CONSTRUCTOR]" << this << "::  gui";

    setupWindow();

    setupMainConsole();
    setupUartConsole();

    setupFpgaCtrl();
    setupThreadProcess();

    setupI2C();
    setupSPI();
    setupPWM();
    setupDma();
    setupFifo();

    setupSeparators();
}

gui::~gui()
{
    qDebug() << "[MAIN] [DESTRUCTOR]" << this << ":: gui ";

    shutdownUart();

    if(false == m_threadKill)
    {
        shutdownThread();
    }
}

void gui::setupWindow()
{
    setWindowTitle("IceNET Platform");
    setFixedSize(dev.xWindow, dev.yWindow);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
    {
        m_phase += 0.1;  // Adjust speed here
        update();      // Triggers paintEvent
    });
    timer->start(30);  // Redraw every 30ms (~33 FPS)
}

void gui::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    double scale = 25.0;
    double step = 0.05;
    double t_max = (dev.xWindow - 800 - scale - dev.xGap*4) / scale;

    int x_offset = 800 + scale + dev.xGap;
    int y_offset = scale*2 + dev.xGap;

    QRect graphArea(x_offset, 0, static_cast<int>(scale * t_max), height());
    if (!event->rect().intersects(graphArea))
        return;

    // ========== SINE + COSINE ==========
    QPen sinPen(Qt::blue, 2);
    QPen cosPen(Qt::darkGreen, 2);
    QPen axisPen(Qt::darkGray, 2, Qt::DashLine);
    QPen highlightPen(Qt::red, 4);

    // Axes
    painter.setPen(axisPen);
    painter.drawLine(QPoint(x_offset, y_offset), QPoint(x_offset + static_cast<int>(scale * t_max), y_offset)); // X
    painter.drawLine(QPoint(x_offset, y_offset - static_cast<int>(scale * 1.5)), QPoint(x_offset, y_offset + static_cast<int>(scale * 1.5))); // Y
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 8));
    painter.drawText(x_offset - 20, y_offset - static_cast<int>(scale * 1.5) - 5, "y(t)");
    painter.drawText(x_offset + static_cast<int>(scale * t_max) + 5, y_offset + 15, "t");

    // Draw sine
    painter.setPen(sinPen);
    QPoint lastPoint;
    bool first = true;
    double lastT = 0.0, lastSin = 0.0;
    for (double t = 0; t <= t_max; t += step) {
        int x = x_offset + static_cast<int>(scale * t);
        double yVal = sin(t + m_phase);
        int y = y_offset - static_cast<int>(scale * yVal);
        QPoint pt(x, y);
        if (!first) painter.drawLine(lastPoint, pt);
        lastPoint = pt;
        first = false;
        lastT = t;
        lastSin = yVal;
    }

    // Draw cosine
    painter.setPen(cosPen);
    lastPoint = QPoint();
    first = true;
    double lastCos = 0.0;
    for (double t = 0; t <= t_max; t += step) {
        int x = x_offset + static_cast<int>(scale * t);
        double yVal = cos(t + m_phase);
        int y = y_offset - static_cast<int>(scale * yVal);
        QPoint pt(x, y);
        if (!first) painter.drawLine(lastPoint, pt);
        lastPoint = pt;
        first = false;
        lastCos = yVal;
    }

    // Highlight latest point
    painter.setPen(highlightPen);
    int x_last = x_offset + static_cast<int>(scale * lastT);
    int y_sin = y_offset - static_cast<int>(scale * lastSin);
    int y_cos = y_offset - static_cast<int>(scale * lastCos);
    painter.drawEllipse(QPoint(x_last, y_sin), 4, 4);
    painter.drawEllipse(QPoint(x_last, y_cos), 4, 4);

    // Data box for sine + cosine
    QRect dataBox1(x_offset + dev.xGap*2, 80 + dev.yGap*2, 180, 50);
    painter.setBrush(QColor(240, 240, 240)); /* RGB Colour */
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(dataBox1);
    painter.setFont(QFont("Arial", 10));
    painter.setPen(Qt::black);
    QString data1 = QString("y(t) = sin(%1) = %2\ny(t) = cos(%1) = %3")
                        .arg(lastT + m_phase, 0, 'f', 2)
                        .arg(lastSin, 0, 'f', 2)
                        .arg(lastCos, 0, 'f', 2);
    painter.drawText(dataBox1.adjusted(10, 10, -10, -10), Qt::AlignLeft | Qt::AlignTop, data1);
}

void gui::setupMainConsole()
{
    m_mainConsoleOutput = new QPlainTextEdit(this);
    m_mainConsoleOutput->setReadOnly(true);
    m_mainConsoleOutput->setGeometry(console.xPosition, console.yPosition, console.xSize, console.ySize);
    m_mainConsoleOutput->setPlainText("[INIT] Main Console Initialized...");
}

void gui::setupUartConsole()
{
    /* Rx window */
    m_uartConsoleOutput = new QPlainTextEdit(this);
    m_uartConsoleOutput->setReadOnly(true);
    m_uartConsoleOutput->setGeometry(console.xPosition, dev.yGap*16 + dev.yLogo*3 + dev.yUnit*9, console.xSize, dev.yWindow - dev.yGap*17 - dev.yUnit*10 - dev.yLogo*3);
    m_uartConsoleOutput->setPlainText("[INIT] UART Console Initialized");
    /* Tx bar */
    m_uartInput = new QLineEdit(this);
    m_uartInput->setGeometry(console.xPosition, dev.yWindow - dev.yGap - dev.yUnit, console.xSize, dev.yUnit);
    connect(m_uartInput, &QLineEdit::returnPressed, this, &gui::onUartInput);
    /* Open device */
    openUart();
}

void gui::setupFpgaCtrl()
{
    QLabel *reset_label = new QLabel("CTRL", this);
    QFont reset_labelFont;
    reset_labelFont.setPointSize(30);
    reset_labelFont.setItalic(true);
    reset_labelFont.setBold(true);
    reset_label->setFont(reset_labelFont);
    reset_label->setGeometry(dev.xGap*5 + dev.xText + dev.xUnit*2, dev.yGap, dev.xLogo, dev.yLogo);

    QPushButton *resetButton = new QPushButton("RESET", this);
    resetButton->setGeometry(dev.xGap*5 + dev.xText + dev.xUnit*2, dev.yGap*2 + dev.yLogo, dev.xUnit*2, dev.yUnit);
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
    connect(resetButton, &QPushButton::clicked, this, [this]()
    {
        if(NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
        }
        else
        {
            m_instanceDroneCtrl->getCommanderInstance()->sendCommand(CMD_FPGA_RESET);
        }
    });

    QPushButton *offloadButton = new QPushButton("OFFLOAD", this);
    offloadButton->setGeometry(dev.xGap*5 + dev.xText + dev.xUnit*2, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xUnit*2, dev.yUnit);
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
    connect(offloadButton, &QPushButton::clicked, this, [this]()
    {
        if(NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
        }
        else
        {
            interruptVector_execute(VECTOR_OFFLOAD);
        }
    });

    QPushButton *enableButton = new QPushButton("ENABLE", this);
    enableButton->setGeometry(dev.xGap*5 + dev.xText + dev.xUnit*2, dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xUnit*2, dev.yUnit);

    enableButton->setStyleSheet(
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

    connect(enableButton, &QPushButton::clicked, this, [this, enableButton]()
    {
        if (NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
            return;
        }

        if (m_isPulseControllerEnabled)
        {
            enableButton->setText("DISABLE");
            enableButton->setStyleSheet(
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

            interruptVector_execute(VECTOR_ENABLE);
        }
        else
        {
            enableButton->setText("ENABLE");
            enableButton->setStyleSheet(
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

            interruptVector_execute(VECTOR_DISABLE);
        }

        // Toggle state
        m_isPulseControllerEnabled = !m_isPulseControllerEnabled;
    });

    QPushButton *startButton = new QPushButton("START", this);
    startButton->setGeometry(dev.xGap*6 + dev.xText + dev.xUnit*4, dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xUnit*2, dev.yUnit);

    startButton->setStyleSheet(
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

    connect(startButton, &QPushButton::clicked, this, [this, startButton]()
    {
        if (NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
            return;
        }

        if (m_isStartAcquisition)
        {
            startButton->setText("STOP");
            startButton->setStyleSheet(
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

            interruptVector_execute(VECTOR_START);
        }
        else
        {
            startButton->setText("START");
            startButton->setStyleSheet(
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

            interruptVector_execute(VECTOR_STOP);
        }

        // Toggle state
        m_isStartAcquisition = !m_isStartAcquisition;
    });

    QPushButton *pulseButton = new QPushButton("PULSE", this);
    pulseButton->setGeometry(dev.xGap*7 + dev.xText + dev.xUnit*6, dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xUnit*2, dev.yUnit);
    pulseButton->setStyleSheet(
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
    connect(pulseButton, &QPushButton::clicked, this, [this]()
    {
        if(NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
        }
        else
        {
            interruptVector_execute(VECTOR_PULSE);
        }
    });

    QPushButton *dataButton = new QPushButton("DATA", this);
    dataButton->setGeometry(dev.xGap*7 + dev.xText + dev.xUnit*6, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xUnit*2, dev.yUnit);
    dataButton->setStyleSheet(
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
    connect(dataButton, &QPushButton::clicked, this, [this]()
    {
        if(NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
        }
        else
        {
            interruptVector_execute(VECTOR_DATA);
        }
    });

    QPushButton *f1Button = new QPushButton("F1", this);
    f1Button->setGeometry(dev.xGap*6 + dev.xText + dev.xUnit*4, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xUnit*2, dev.yUnit);
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
    connect(f1Button, &QPushButton::clicked, this, [this]()
    {
        if(NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
        }
        else
        {
            interruptVector_execute(VECTOR_F1);
        }
    });

    QPushButton *f2Button = new QPushButton("F2", this);
    f2Button->setGeometry(dev.xGap*6 + dev.xText + dev.xUnit*4, dev.yGap*2 + dev.yLogo, dev.xUnit*2, dev.yUnit);
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
    connect(f2Button, &QPushButton::clicked, this, [this]()
    {
        if(NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
        }
        else
        {
            interruptVector_execute(VECTOR_F2);
        }
    });

    QPushButton *f3Button = new QPushButton("F3", this);
    f3Button->setGeometry(dev.xGap*7 + dev.xText + dev.xUnit*6, dev.yGap*2 + dev.yLogo, dev.xUnit*2, dev.yUnit);
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
    connect(f3Button, &QPushButton::clicked, this, [this]()
    {
        if(NULL == m_instanceDroneCtrl)
        {
            printToMainConsole("[CTL] threadMain is not Running");
        }
        else
        {
            interruptVector_execute(VECTOR_F3);
        }
    });
}

void gui::setupThreadProcess()
{
    QLabel *thread_label = new QLabel("THREAD", this);
    QFont thread_labelFont;
    thread_labelFont.setPointSize(30);
    thread_labelFont.setItalic(true);
    thread_labelFont.setBold(true);
    thread_label->setFont(thread_labelFont);
    thread_label->setGeometry(800 - dev.xGap*2 - dev.xUnit*4 , dev.yGap, dev.xLogo, dev.yLogo);

    QPushButton *connectButton = new QPushButton("INITIALIZE", this);
    connectButton->setGeometry(800 - dev.xGap*2 - dev.xUnit*4, dev.yGap*2 + dev.yLogo, dev.xUnit*4 + dev.xGap, dev.yUnit);
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
    connect(connectButton, &QPushButton::clicked, this, &gui::initThread);

    QPushButton *terminateButton = new QPushButton("TERMINATE", this);
    terminateButton->setGeometry(800 - dev.xGap*2 - dev.xUnit*4, dev.yGap*3 + dev.yUnit*3, dev.xUnit*4 + dev.xGap, dev.yUnit);
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
    connect(terminateButton, &QPushButton::clicked, this, &gui::shutdownThread);

    QPushButton *debugButton = new QPushButton("DEBUG", this);
    debugButton->setGeometry(800 - dev.xGap*2 - dev.xUnit*4, dev.yGap*4 + dev.yUnit*4, dev.xUnit*4 + dev.xGap, dev.yUnit);
    debugButton->setStyleSheet(
        "QPushButton {"
        "   background-color: purple;"
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
    connect(debugButton, &QPushButton::clicked, this, &gui::debugThread);
}

void gui::setupSeparators()
{
    /* Vertical Separator */
    QFrame *vLine1 = new QFrame(this);
    vLine1->setGeometry(dev.xGap*4 + dev.xText + dev.xUnit*2, dev.yGap, dev.separatorWidth , dev.yWindow - dev.yGap*2);
    vLine1->setFrameShape(QFrame::VLine);
    vLine1->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine1 = new QFrame(this);
    hLine1->setGeometry(dev.xGap, dev.yGap*5 + dev.yLogo + dev.yUnit*3, dev.xWindow - dev.xGap*2, dev.separatorWidth);
    hLine1->setFrameShape(QFrame::HLine);
    hLine1->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine2 = new QFrame(this);
    hLine2->setGeometry(dev.xGap, dev.yGap*10 + dev.yLogo*2 + dev.yUnit*6, dev.xGap*3 + dev.xText + dev.xUnit*2, dev.separatorWidth);
    hLine2->setFrameShape(QFrame::HLine);
    hLine2->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine3 = new QFrame(this);
    hLine3->setGeometry(dev.xGap, dev.yGap*15 + dev.yLogo*3 + dev.yUnit*9, dev.xWindow - dev.xGap*2, dev.separatorWidth);
    hLine3->setFrameShape(QFrame::HLine);
    hLine3->setFrameShadow(QFrame::Sunken);
    /* Vertical Separator */
    QFrame *vLine2 = new QFrame(this);
    vLine2->setGeometry(800 - dev.xGap*3 - dev.xUnit*4 , dev.yGap, dev.separatorWidth, dev.yGap*4 + dev.yLogo + dev.yUnit*3);
    vLine2->setFrameShape(QFrame::VLine);
    vLine2->setFrameShadow(QFrame::Sunken);
    /* Vertical Separator */
    QFrame *vLine3 = new QFrame(this);
    vLine3->setGeometry(800, dev.yGap, dev.separatorWidth, dev.yGap*4 + dev.yLogo + dev.yUnit*3);
    vLine3->setFrameShape(QFrame::VLine);
    vLine3->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine4 = new QFrame(this);
    hLine4->setGeometry(dev.xGap, dev.yGap*20 + dev.yLogo*4 + dev.yUnit*12, dev.xGap*3 + dev.xText + dev.xUnit*2, dev.separatorWidth);
    hLine4->setFrameShape(QFrame::HLine);
    hLine4->setFrameShadow(QFrame::Sunken);
}

void gui::setupI2C()
{
    /* I2C :: Row[0] */
    QLabel *i2c_label = new QLabel("I2C", this);
    QFont i2c_labelFont;
    i2c_labelFont.setPointSize(30);
    i2c_labelFont.setItalic(true);
    i2c_labelFont.setBold(true);
    i2c_label->setFont(i2c_labelFont);
    i2c_label->setGeometry(dev.xGap , dev.yGap, dev.xLogo, dev.yLogo);
    /* I2C :: Row[1] */
    QLabel *i2c_addressLabel = new QLabel("Device Address", this);
    i2c_addressLabel->setGeometry(dev.xGap, dev.yGap*2 + dev.yLogo, dev.xText, dev.yUnit);
    m_i2c_addressField = new QLineEdit(this);
    m_i2c_addressField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*2 + dev.yLogo, dev.xUnit, dev.yUnit);
    m_i2c_addressField->setText("0x53");
    QPushButton *i2c_exeButton = new QPushButton("EXE", this);
    i2c_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*2 + dev.yLogo, dev.xUnit, dev.yUnit);
    connect(i2c_exeButton, &QPushButton::clicked, this, &gui::i2c_execute);
    /* I2C :: Row[2] */
    QLabel *i2c_registerLabel = new QLabel("Register Address", this);
    i2c_registerLabel->setGeometry(dev.xGap, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xText, dev.yUnit);
    m_i2c_registerField = new QLineEdit(this);
    m_i2c_registerField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xUnit, dev.yUnit);
    m_i2c_registerField->setText("0x00");
    m_i2c_burstField = new QLineEdit(this);
    m_i2c_burstField->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xUnit, dev.yUnit);
    m_i2c_burstField->setText("0x01");
    /* I2C :: Row[3] */
    QLabel *i2c_dataLabel = new QLabel("Write Data", this);
    i2c_dataLabel->setGeometry(dev.xGap, dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xText, dev.yUnit);
    m_i2c_dataField = new QLineEdit(this);
    m_i2c_dataField->setGeometry(dev.xGap*2 + dev.xText , dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xUnit, dev.yUnit);
    m_i2c_dataField->setText("0x00");
    m_i2c_dataField->setDisabled(true);
    m_i2c_writeTick = new QCheckBox("WR", this);
    m_i2c_writeTick->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xUnit, dev.yUnit);
    connect(m_i2c_writeTick, &QCheckBox::toggled, m_i2c_dataField, &QLineEdit::setEnabled);
}

void gui::setupSPI()
{
    /* SPI :: Row[0] */
    QLabel *spi_label = new QLabel("SPI", this);
    QFont spi_labelFont;
    spi_labelFont.setPointSize(30);
    spi_labelFont.setItalic(true);
    spi_labelFont.setBold(true);
    spi_label->setFont(spi_labelFont);
    spi_label->setGeometry(dev.xGap, dev.yGap*6 + dev.yLogo + dev.yUnit*3, dev.xLogo, dev.yLogo);
    /* SPI :: Row[1] */
    QLabel *spi_addressLabel = new QLabel("FPGA Device ID", this);
    spi_addressLabel->setGeometry(dev.xGap, dev.yGap*7 + dev.yLogo*2 + dev.yUnit*3, dev.xText, dev.yUnit);
    m_spi_addressField = new QLineEdit(this);
    m_spi_addressField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*7 + dev.yLogo*2 + dev.yUnit*3, dev.xUnit, dev.yUnit);
    m_spi_addressField->setText("0x11");
    QPushButton *spi_exeButton = new QPushButton("EXE", this);
    spi_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*7 + dev.yLogo*2 + dev.yUnit*3, dev.xUnit, dev.yUnit);
    connect(spi_exeButton, &QPushButton::clicked, this, &gui::spi_execute);
    /* SPI :: Row[2] */
    QLabel *spi_registerLabel = new QLabel("Register Address", this);
    spi_registerLabel->setGeometry(dev.xGap, dev.yGap*8 + dev.yLogo*2 + dev.yUnit*4, dev.xText, dev.yUnit);
    m_spi_registerField = new QLineEdit(this);
    m_spi_registerField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*8 + dev.yLogo*2 + dev.yUnit*4, dev.xUnit, dev.yUnit);
    m_spi_registerField->setText("0x92");
    m_spi_burstField = new QLineEdit(this);
    m_spi_burstField->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*8 + dev.yLogo*2 + dev.yUnit*4, dev.xUnit, dev.yUnit);
    m_spi_burstField->setText("0x01");
    /* SPI :: Row[3] */
    QLabel *spi_dataLabel = new QLabel("Write Data", this);
    spi_dataLabel->setGeometry(dev.xGap, dev.yGap*9 + dev.yLogo*2 + dev.yUnit*5, dev.xText, dev.yUnit);
    m_spi_dataField = new QLineEdit(this);
    m_spi_dataField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*9 + dev.yLogo*2 + dev.yUnit*5, dev.xUnit, dev.yUnit);
    m_spi_dataField->setText("0x00");
    m_spi_dataField->setDisabled(true);
    m_spi_writeTick = new QCheckBox("WR", this);
    m_spi_writeTick->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*9 + dev.yLogo*2 + dev.yUnit*5, dev.xUnit, dev.yUnit);
    connect(m_spi_writeTick, &QCheckBox::toggled, m_spi_dataField, &QLineEdit::setEnabled);
    /**
     *
     * connect(sender, SIGNAL(signalName(parameters)), receiver, SLOT(slotName(parameters)));
     *
     * sender → The object emitting the signal
     * signalName(parameters) → The signal emitted when an event occurs
     * receiver → The object that receives and processes the signal
     * slotName(parameters) → The function executed when the signal is received
     *
     * connect(m_spi_writeTick, &QCheckBox::toggled, m_spi_burstField, &QLineEdit::setDisabled);
     *
     */
    connect(m_spi_writeTick, &QCheckBox::toggled, this, [=](bool checked)
    {
        m_spi_burstField->setDisabled(checked); // Disable burst field when checked
        if (checked)
        {
            m_spi_burstField->setText("0x01"); // Ensure value is "0x01" when checked
        }
    });
}

void gui::setupPWM()
{
    /* PWM :: Row[0] */
    QLabel *pwm_label = new QLabel("PWM", this);
    QFont pwm_labelFont;
    pwm_labelFont.setPointSize(30);
    pwm_labelFont.setItalic(true);
    pwm_labelFont.setBold(true);
    pwm_label->setFont(pwm_labelFont);
    pwm_label->setGeometry(dev.xGap, dev.yGap*11 + dev.yLogo*2 + dev.yUnit*6, dev.xLogo, dev.yLogo);
    /* PWM :: Row[1] */
    QLabel *pwm_speedLabel = new QLabel("Speed [Hex]", this);
    pwm_speedLabel->setGeometry(dev.xGap, dev.yGap*12 + dev.yLogo*3 + dev.yUnit*6, dev.xText, dev.yUnit);
    m_pwm_dataField = new QLineEdit(this);
    m_pwm_dataField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*12 + dev.yLogo*3 + dev.yUnit*6, dev.xUnit, dev.yUnit);
    m_pwm_dataField->setText("0x00");
    QPushButton *pwm_exeButton = new QPushButton("EXE", this);
    pwm_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*12 + dev.yLogo*3 + dev.yUnit*6, dev.xUnit, dev.yUnit);
    connect(pwm_exeButton, &QPushButton::clicked, this, [this]()
    {
        pwm_execute(PWM_EXE);
    });
    /* PWM :: Row[2] */
    QPushButton *pwm_upButton = new QPushButton("UP", this);
    pwm_upButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*13 + dev.yLogo*3 + dev.yUnit*7, dev.xUnit, dev.yUnit);
    connect(pwm_upButton, &QPushButton::clicked, this, [this]()
    {
        pwm_execute(PWM_UP);
    });
    /* PWM :: Row[3] */
    QPushButton *pwm_downButton = new QPushButton("DOWN", this);
    pwm_downButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*14 + dev.yLogo*3 + dev.yUnit*8, dev.xUnit, dev.yUnit);
    connect(pwm_downButton, &QPushButton::clicked, this, [this]()
    {
        pwm_execute(PWM_DOWN);
    });
}

void gui::setupDma()
{
    /* DMA :: Row[0] */
    QLabel *i2c_label = new QLabel("DMA", this);
    QFont i2c_labelFont;
    i2c_labelFont.setPointSize(30);
    i2c_labelFont.setItalic(true);
    i2c_labelFont.setBold(true);
    i2c_label->setFont(i2c_labelFont);
    i2c_label->setGeometry(dev.xGap, dev.yGap*16 + dev.yLogo*3 + dev.yUnit*9, dev.xLogo, dev.yLogo);
    /* DMA :: Row[1] */
    QLabel *dma_customLabel = new QLabel("[FPGA->CPU] Custom", this);
    dma_customLabel->setGeometry(dev.xGap, dev.yGap*16 + dev.yLogo*4 + dev.yUnit*9, dev.xText, dev.yUnit);
    m_dmaCustom_dataField = new QLineEdit(this);
    m_dmaCustom_dataField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*16 + dev.yLogo*4 + dev.yUnit*9, dev.xUnit, dev.yUnit);
    m_dmaCustom_dataField->setText("0x01");
    QPushButton *dma_custom_exeButton = new QPushButton("EXE", this);
    dma_custom_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*16 + dev.yLogo*4 + dev.yUnit*9, dev.xUnit, dev.yUnit);
    connect(dma_custom_exeButton, &QPushButton::clicked, this, [this]()
    {
        dma_execute(CMD_DMA_CUSTOM);
    });
    /* DMA :: Row[2] */
    QLabel *dma_singleLabel = new QLabel("[FPGA->CPU] Single", this);
    dma_singleLabel->setGeometry(dev.xGap, dev.yGap*17 + dev.yLogo*4 + dev.yUnit*10, dev.xText, dev.yUnit);
    QPushButton *dma_single_exeButton = new QPushButton("EXE", this);
    dma_single_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*17 + dev.yLogo*4 + dev.yUnit*10, dev.xUnit, dev.yUnit);
    connect(dma_single_exeButton, &QPushButton::clicked, this, [this]()
    {
        dma_execute(CMD_DMA_SINGLE);
    });
    /* DMA :: Row[3] */
    QLabel *i2c_registerLabel = new QLabel("[FPGA->CPU] Sensor", this);
    i2c_registerLabel->setGeometry(dev.xGap, dev.yGap*18 + dev.yLogo*4 + dev.yUnit*11, dev.xText, dev.yUnit);
    QPushButton *dmaSensor_exeButton = new QPushButton("EXE", this);
    dmaSensor_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*18 + dev.yLogo*4 + dev.yUnit*11, dev.xUnit, dev.yUnit);
    connect(dmaSensor_exeButton, &QPushButton::clicked, this, [this]()
    {
        dma_execute(CMD_DMA_SENSOR);
    });
}

void gui::setupFifo()
{
    /* FIFO :: Row[0] */
    QLabel *i2c_label = new QLabel("FIFO", this);
    QFont i2c_labelFont;
    i2c_labelFont.setPointSize(30);
    i2c_labelFont.setItalic(true);
    i2c_labelFont.setBold(true);
    i2c_label->setFont(i2c_labelFont);
    i2c_label->setGeometry(dev.xGap, dev.yGap*21 + dev.yLogo*4 + dev.yUnit*12, dev.xLogo, dev.yLogo);
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
        case VECTOR_RESERVED:   return "VECTOR_RESERVED";
        case VECTOR_OFFLOAD:    return "VECTOR_OFFLOAD";    /* FIFO Offload chain */
        case VECTOR_ENABLE:     return "VECTOR_ENABLE";     /* Enable Pulse Controllers */
        case VECTOR_DISABLE:    return "VECTOR_DISABLE";    /* Disable Pulse Controllers */
        case VECTOR_START:      return "VECTOR_START";      /* Start Measurement Acquisition */
        case VECTOR_STOP:       return "VECTOR_STOP";       /* Stop Measurement Acquisition */
        case VECTOR_PULSE:      return "VECTOR_PULSE";      /* Single 20ns Pulse :: To be connected anywhere in FPGA */
        case VECTOR_DATA:       return "VECTOR_DATA";       /* Offload data from sensor FIFO */
        case VECTOR_F1:         return "VECTOR_F1";         /* Undefined Function F1 */
        case VECTOR_F2:         return "VECTOR_F2";         /* Undefined Function F2 */
        case VECTOR_F3:         return "VECTOR_F3";         /* Undefined Function F3 */
        case VECTOR_UNUSED_11:  return "VECTOR_UNUSED_11";
        case VECTOR_UNUSED_12:  return "VECTOR_UNUSED_12";
        case VECTOR_UNUSED_13:  return "VECTOR_UNUSED_13";
        case VECTOR_UNUSED_14:  return "VECTOR_UNUSED_14";
        case VECTOR_UNUSED_15:  return "VECTOR_UNUSED_15";
        default:                return "UNKNOWN_VECTOR";
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
    // 0000 :: VECTOR_RESERVED
    // 0001 :: VECTOR_OFFLOAD
    // 0010 :: VECTOR_ENABLE
    // 0011 :: VECTOR_DISABLE
    // 0100 :: VECTOR_START
    // 0101 :: VECTOR_STOP
    // 0110 :: VECTOR_PULSE
    // 0111 ::
    // 1000 ::
    // 1001 ::
    // 1010 ::
    // 1011 ::
    // 1100 ::
    // 1101 ::
    // 1110 ::
    // 1111 ::
    //
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

    *m_IO_GuiState = IO_COM_WRITE_ONLY;
    printToMainConsole("[INT] Done -> " + QString::fromStdString(vectorToString(type)));
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
        case CMD_FPGA_RESET:    return "CMD_FPGA_RESET";
        default:                return "UNKNOWN_CMD";
    }
}

void gui::dma_execute(commandType cmd)
{
    if(NULL == m_instanceDroneCtrl)
    {
        printToMainConsole("[DMA] threadMain is not Running");
    }
    else
    {
        printToMainConsole("[DMA] Send DMA Command to Kernel -> " + QString::fromStdString(cmdToString(cmd)));
        if(CMD_DMA_CUSTOM == cmd)
        {
            bool ok;
            QString dataText = m_dmaCustom_dataField->text();
            uint8_t dmaSize = static_cast<uint8_t>(dataText.toUInt(&ok, 16));

            m_instanceDroneCtrl->getCommanderInstance()->setDmaCustom(dmaSize);
            m_instanceDroneCtrl->getCommanderInstance()->sendCommand(cmd);
        }
        else if(CMD_DMA_SINGLE == cmd || CMD_DMA_SENSOR == cmd)
        {
            m_instanceDroneCtrl->getCommanderInstance()->sendCommand(cmd);
        }
        else
        {
            printToMainConsole("[DMA] Wrong DMA Command");
        }
    }
}

void gui::i2c_execute()
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(m_threadKill)
    {
        printToMainConsole("[I2C] threadMain is Down");
    }
    else if(!m_isKernelConnected)
    {
        printToMainConsole("[I2C] Kernel Communication is Down");
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
        *m_IO_GuiState = IO_COM_WRITE_ONLY;

        /* Wait for Kerenl to send data to FPGA */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printToMainConsole("[I2C] Done");
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

    if(m_threadKill)
    {
        printToMainConsole("[SPI] threadMain is Down");
    }
    else if(!m_isKernelConnected)
    {
        printToMainConsole("[SPI] Kernel Communication is Down");
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
        *m_IO_GuiState = IO_COM_WRITE_ONLY;

        /* Wait for Kerenl to send data to FPGA */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printToMainConsole("[SPI] Done");
    }
}

void gui::pwm_execute(pwmType type)
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(m_threadKill)
    {
        printToMainConsole("[PWM] threadMain is Down");
    }
    else if(!m_isKernelConnected)
    {
        printToMainConsole("[PWM] Kernel Communication is Down");
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
                    printToMainConsole("[PWM] Execute");
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
                    printToMainConsole("[PWM] Up");
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
                    printToMainConsole("[PWM] Down");
                    break;

                default:
                    printToMainConsole("[PWM] Unknown type of operation");
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
        *m_IO_GuiState = IO_COM_WRITE_ONLY;

        /* Wait for Kerenl to send data to FPGA */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printToMainConsole("[PWM] Done");
    }
}

void gui::openUart()
{
    m_serialPort = new QSerialPort(this);

    m_uartPortName = "/dev/ttyTHS1";
    m_serialPort->setPortName(m_uartPortName);
#if 0
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

#if 0
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

void gui::initThread()
{
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadMain.joinable())
    {
        printToMainConsole("[THR] threadMain is already running.");
        return;
    }

    printToMainConsole("[THR] Initialize threadMain");

    m_threadKill = false;
    m_threadMain = std::thread(&gui::threadMain, this);

    qDebug() << "[INIT] [THR] Initialize threadMain";
}

void gui::shutdownThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        printToMainConsole("[THR] threadMain is already marked for shutdown.");
        return;
    }

    printToMainConsole("[THR] Shutdown threadMain");

    m_threadKill = true;

    if (m_threadMain.joinable())
    {
        m_threadMain.join();
        printToMainConsole("[THR] threadMain has been shut down.");
    }

    qDebug() << "[EXIT] [THR] Terminate threadMain";
}

void gui::debugThread()
{
    /**
     *
     * TO BE
     * CONSIDERED
     * LATE#R
     *
     */
    printToMainConsole("[THR] Debug threadMain");
    qDebug() << "[CTRL] [THR] Debug threadMain";
}

void gui::threadMain()
{
    /**
     *
     * Smart pointer for auto Heap
     * allocation and deallocation
     *
     */

    /* 1st :: Make unique DroneCtrl */
    m_instanceDroneCtrl = std::make_unique<DroneCtrl>();
    /* 2st :: Initialise DroneCtrl pointers */
    m_instanceDroneCtrl->initPointers();
    /* 3nd :: Push transfer pointers to Commander */
    m_instanceDroneCtrl->getCommanderInstance()->setTransferPointers(m_Rx_GuiVector, m_Tx_GuiVector, m_IO_GuiState);
    /* 4th :: Init RamDisk Commander */
    m_instanceDroneCtrl->initKernelComms();

    while (false == m_threadKill)
    {
        m_instanceDroneCtrl->droneCtrlMain();

        if (true == m_instanceDroneCtrl->isKilled())
        {
            /* TODO :: Button stays pressed at Thread Termination */
            interruptVector_execute(VECTOR_DISABLE);
            m_isKernelConnected = false;
            break;
        }
        else
        {
            m_isKernelConnected = true;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    m_instanceDroneCtrl->getCommanderInstance()->sendCommand(CMD_FPGA_RESET);
    m_instanceDroneCtrl->getCommanderInstance()->sendCommand(CMD_RAMDISK_CLEAR);
    m_instanceDroneCtrl->shutdownKernelComms();
    m_instanceDroneCtrl.reset(); // Reset the unique_ptr to call the destructor
}
