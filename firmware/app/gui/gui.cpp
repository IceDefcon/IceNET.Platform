/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include "gui.h"

static const deviceType dev =
{
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
    .yPosition = dev.yGap,                              // Top
    .xSize = 800 - console.xPosition - dev.xGap,        // Obvious
    .ySize = dev.yGap*14 + dev.yLogo*3 + dev.yUnit*9+1, // Last Horizontal Separator
};

gui::gui() :
m_threadKill(true),
m_isKernelConnected(false),
m_Rx_GuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_Tx_GuiVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_IO_GuiState(std::make_shared<ioStateType>(IO_IDLE))
{
    qDebug() << "[MAIN] [CONSTRUCTOR]" << this << "::  gui";

    setupWindow();
    setupConsole();

    setupI2C();
    setupSPI();
    setupPWM();

    setupSeparators();
    setupProcess();
}

gui::~gui()
{
    qDebug() << "[MAIN] [DESTRUCTOR]" << this << ":: gui ";
    if(false == m_threadKill)
    {
        shutdownThread();
    }
}

void gui::setupWindow()
{
    setWindowTitle("IceNET Platform");
    setFixedSize(800, 600);
}

void gui::setupConsole()
{
    m_consoleOutput = new QPlainTextEdit(this);
    m_consoleOutput->setReadOnly(true);
    m_consoleOutput->setGeometry(console.xPosition, console.yPosition, console.xSize, console.ySize);
    m_consoleOutput->setPlainText("Console Initialized...\n");
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
    m_i2c_addressField->setText("0x69");
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
    m_spi_registerField->setText("0x00");
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







void gui::setupSeparators()
{
    /* Vertical Separator */
    QFrame *vLine1 = new QFrame(this);
    vLine1->setGeometry(dev.xGap*4 + dev.xText + dev.xUnit*2, dev.yGap, dev.separatorWidth , dev.yGap*14 + dev.yLogo*3 + dev.yUnit*9);
    vLine1->setFrameShape(QFrame::VLine);
    vLine1->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine1 = new QFrame(this);
    hLine1->setGeometry(dev.xGap, dev.yGap*5 + dev.yLogo + dev.yUnit*3, dev.xGap*3 + dev.xText + dev.xUnit*2, dev.separatorWidth);
    hLine1->setFrameShape(QFrame::HLine);
    hLine1->setFrameShadow(QFrame::Sunken);
    /* Horizontal Separator */
    QFrame *hLine2 = new QFrame(this);
    hLine2->setGeometry(dev.xGap, dev.yGap*10 + dev.yLogo*2 + dev.yUnit*6, dev.xGap*3 + dev.xText + dev.xUnit*2, dev.separatorWidth);
    hLine2->setFrameShape(QFrame::HLine);
    hLine2->setFrameShadow(QFrame::Sunken);

    /* Horizontal Separator */
    QFrame *hLine3 = new QFrame(this);
    hLine3->setGeometry(dev.xGap, dev.yGap*15 + dev.yLogo*3 + dev.yUnit*9, dev.xGap*3 + dev.xText + dev.xUnit*2, dev.separatorWidth);
    hLine3->setFrameShape(QFrame::HLine);
    hLine3->setFrameShadow(QFrame::Sunken);
}

void gui::setupProcess()
{
    QPushButton *connectButton = new QPushButton("CONNECT", this);
    connectButton->setGeometry(dev.xGap, dev.yGap * 16 + dev.yLogo * 3 + dev.yUnit * 9,dev.xUnit * 5 + dev.xGap, dev.yUnit * 2);
    connectButton->setStyleSheet(
        "QPushButton {"
        "   background-color: green;"
        "   color: white;"
        "   font-size: 18px;"
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
    terminateButton->setGeometry(dev.xGap, dev.yGap * 17 + dev.yLogo * 3 + dev.yUnit * 11,dev.xUnit * 5 + dev.xGap, dev.yUnit * 2);
    terminateButton->setStyleSheet(
        "QPushButton {"
        "   background-color: red;"
        "   color: white;"
        "   font-size: 18px;"
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
}

void gui::setDeadCommand()
{
    /* Dead Code :: In case if something happen */
    (*m_Tx_GuiVector)[0] = 0xDE;
    (*m_Tx_GuiVector)[1] = 0xAD;
    (*m_Tx_GuiVector)[2] = 0xC0;
    (*m_Tx_GuiVector)[3] = 0xD3;
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

void gui::i2c_execute()
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(m_threadKill)
    {
        printToConsole("[I2C] threadMain is Down");
    }
    else if(!m_isKernelConnected)
    {
        printToConsole("[I2C] Kernel Communication is Down");
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
        *m_IO_GuiState = IO_COM_WRITE;

        printToConsole("[I2C] Done");
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
        printToConsole("[SPI] threadMain is Down");
    }
    else if(!m_isKernelConnected)
    {
        printToConsole("[SPI] Kernel Communication is Down");
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
        *m_IO_GuiState = IO_COM_WRITE;

        printToConsole("[SPI] Done");
    }
}

void gui::pwm_execute(pwmType type)
{
    /* Dead Command :: In case if something happen */
    setDeadCommand();

    if(m_threadKill)
    {
        printToConsole("[PWM] threadMain is Down");
    }
    else if(!m_isKernelConnected)
    {
        printToConsole("[PWM] Kernel Communication is Down");
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
                    printToConsole("[PWM] Execute");
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
                    printToConsole("[PWM] Up");
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
                    printToConsole("[PWM] Down");
                    break;

                default:
                    printToConsole("[PWM] Unknown type of operation");
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
        *m_IO_GuiState = IO_COM_WRITE;

        printToConsole("[PWM] Done");
    }
}

void gui::printToConsole(const QString &message)
{
    m_consoleOutput->appendPlainText(message);
}

void gui::initThread()
{
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadMain.joinable())
    {
        printToConsole("[THR] threadMain is already running.");
        return;
    }

    printToConsole("[THR] Initialize threadMain");

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
        printToConsole("[THR] threadMain is already marked for shutdown.");
        return;
    }

    printToConsole("[THR] Shutdown threadMain");

    m_threadKill = true;

    if (m_threadMain.joinable())
    {
        m_threadMain.join();
        printToConsole("[THR] threadMain has been shut down.");
    }

    qDebug() << "[EXIT] [THR] Terminate threadMain";
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

    m_instanceDroneCtrl->shutdownKernelComms();
    m_instanceDroneCtrl.reset(); // Reset the unique_ptr to call the destructor
}

