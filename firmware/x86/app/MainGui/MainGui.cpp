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
            printToMainConsole("$ Drone Control is Down");
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
