#include <QPlainTextEdit>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPainter>
#include <QWidget>
#include <QDebug>
#include <QLabel>
#include <QFont>

typedef struct
{
    uint32_t xGap;
    uint32_t yGap;
    uint32_t xLogo;
    uint32_t yLogo;
    uint32_t xText;
    uint32_t xUnit;
    uint32_t yUnit;
    uint32_t separatorLength;
    uint32_t separatorWidth;
} deviceType;

typedef struct
{
    uint32_t xPosition;
    uint32_t yPosition;
    uint32_t xSize;
    uint32_t ySize;
} consoleType;

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

class WindowDesign : public QWidget
{
    QPlainTextEdit *consoleOutput;
    QLineEdit *i2c_addressField, *i2c_registerField, *i2c_burstField, *i2c_dataField;
    QLineEdit *spi_addressField, *spi_registerField, *spi_burstField, *spi_dataField;
    QLineEdit *pwm_dataField;

public:

    WindowDesign()
    {
        setWindowTitle("IceNET Platform");
        setFixedSize(800, 600);

        consoleOutput = new QPlainTextEdit(this);
        consoleOutput->setReadOnly(true);
        consoleOutput->setGeometry(console.xPosition, console.yPosition, console.xSize, console.ySize);
        consoleOutput->setPlainText("Console Initialized...\n");

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
        i2c_addressField = new QLineEdit(this);
        i2c_addressField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*2 + dev.yLogo, dev.xUnit, dev.yUnit);
        i2c_addressField->setText("0x69");
        QPushButton *i2c_exeButton = new QPushButton("EXE", this);
        i2c_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*2 + dev.yLogo, dev.xUnit, dev.yUnit);
        connect(i2c_exeButton, &QPushButton::clicked, this, &WindowDesign::i2c_execute);
        /* I2C :: Row[2] */
        QLabel *i2c_registerLabel = new QLabel("Register Address", this);
        i2c_registerLabel->setGeometry(dev.xGap, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xText, dev.yUnit);
        i2c_registerField = new QLineEdit(this);
        i2c_registerField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xUnit, dev.yUnit);
        i2c_registerField->setText("0x00");
        i2c_burstField = new QLineEdit(this);
        i2c_burstField->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*3 + dev.yLogo + dev.yUnit, dev.xUnit, dev.yUnit);
        i2c_burstField->setText("0x01");
        /* I2C :: Row[3] */
        QLabel *i2c_dataLabel = new QLabel("Write Data", this);
        i2c_dataLabel->setGeometry(dev.xGap, dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xText, dev.yUnit);
        i2c_dataField = new QLineEdit(this);
        i2c_dataField->setGeometry(dev.xGap*2 + dev.xText , dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xUnit, dev.yUnit);
        i2c_dataField->setText("0x00");
        i2c_dataField->setDisabled(true);
        QCheckBox *i2c_writeTick = new QCheckBox("WR", this);
        i2c_writeTick->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*4 + dev.yLogo + dev.yUnit*2, dev.xUnit, dev.yUnit);
        connect(i2c_writeTick, &QCheckBox::toggled, i2c_dataField, &QLineEdit::setEnabled);

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

        /* SPI :: Row[0] */
        QLabel *spi_label = new QLabel("SPI", this);
        QFont spi_labelFont;
        spi_labelFont.setPointSize(30);
        spi_labelFont.setItalic(true);
        spi_labelFont.setBold(true);
        spi_label->setFont(i2c_labelFont);
        spi_label->setGeometry(dev.xGap, dev.yGap*6 + dev.yLogo + dev.yUnit*3, dev.xLogo, dev.yLogo);
        /* SPI :: Row[1] */
        QLabel *spi_addressLabel = new QLabel("Device Address", this);
        spi_addressLabel->setGeometry(dev.xGap, dev.yGap*7 + dev.yLogo*2 + dev.yUnit*3, dev.xText, dev.yUnit);
        spi_addressField = new QLineEdit(this);
        spi_addressField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*7 + dev.yLogo*2 + dev.yUnit*3, dev.xUnit, dev.yUnit);
        spi_addressField->setText("0x11");
        QPushButton *spi_exeButton = new QPushButton("EXE", this);
        spi_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*7 + dev.yLogo*2 + dev.yUnit*3, dev.xUnit, dev.yUnit);
        connect(spi_exeButton, &QPushButton::clicked, this, &WindowDesign::spi_execute);
        /* SPI :: Row[2] */
        QLabel *spi_registerLabel = new QLabel("Register Address", this);
        spi_registerLabel->setGeometry(dev.xGap, dev.yGap*8 + dev.yLogo*2 + dev.yUnit*4, dev.xText, dev.yUnit);
        spi_registerField = new QLineEdit(this);
        spi_registerField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*8 + dev.yLogo*2 + dev.yUnit*4, dev.xUnit, dev.yUnit);
        spi_registerField->setText("0x00");
        spi_burstField = new QLineEdit(this);
        spi_burstField->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*8 + dev.yLogo*2 + dev.yUnit*4, dev.xUnit, dev.yUnit);
        spi_burstField->setText("0x01");
        /* SPI :: Row[3] */
        QLabel *spi_dataLabel = new QLabel("Write Data", this);
        spi_dataLabel->setGeometry(dev.xGap, dev.yGap*9 + dev.yLogo*2 + dev.yUnit*5, dev.xText, dev.yUnit);
        spi_dataField = new QLineEdit(this);
        spi_dataField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*9 + dev.yLogo*2 + dev.yUnit*5, dev.xUnit, dev.yUnit);
        spi_dataField->setText("0x00");
        spi_dataField->setDisabled(true);
        QCheckBox *spi_writeTick = new QCheckBox("WR", this);
        spi_writeTick->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*9 + dev.yLogo*2 + dev.yUnit*5, dev.xUnit, dev.yUnit);
        connect(spi_writeTick, &QCheckBox::toggled, spi_dataField, &QLineEdit::setEnabled);

        /* Horizontal Separator */
        QFrame *hLine2 = new QFrame(this);
        hLine2->setGeometry(dev.xGap, dev.yGap*10 + dev.yLogo*2 + dev.yUnit*6, dev.xGap*3 + dev.xText + dev.xUnit*2, dev.separatorWidth);
        hLine2->setFrameShape(QFrame::HLine);
        hLine2->setFrameShadow(QFrame::Sunken);

        /* PWM :: Row[0] */
        QLabel *pwm_label = new QLabel("PWM", this);
        QFont pwm_labelFont;
        pwm_labelFont.setPointSize(30);
        pwm_labelFont.setItalic(true);
        pwm_labelFont.setBold(true);
        pwm_label->setFont(i2c_labelFont);
        pwm_label->setGeometry(dev.xGap, dev.yGap*11 + dev.yLogo*2 + dev.yUnit*6, dev.xLogo, dev.yLogo);
        /* PWM :: Row[1] */
        QLabel *pwm_speedLabel = new QLabel("Speed [Hex]", this);
        pwm_speedLabel->setGeometry(dev.xGap, dev.yGap*12 + dev.yLogo*3 + dev.yUnit*6, dev.xText, dev.yUnit);
        pwm_dataField = new QLineEdit(this);
        pwm_dataField->setGeometry(dev.xGap*2 + dev.xText, dev.yGap*12 + dev.yLogo*3 + dev.yUnit*6, dev.xUnit, dev.yUnit);
        pwm_dataField->setText("0x00");
        QPushButton *pwm_exeButton = new QPushButton("EXE", this);
        pwm_exeButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*12 + dev.yLogo*3 + dev.yUnit*6, dev.xUnit, dev.yUnit);
        connect(pwm_exeButton, &QPushButton::clicked, this, &WindowDesign::pwm_execute);
        /* PWM :: Row[2] */
        QPushButton *pwm_upButton = new QPushButton("UP", this);
        pwm_upButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*13 + dev.yLogo*3 + dev.yUnit*7, dev.xUnit, dev.yUnit);
        connect(pwm_upButton, &QPushButton::clicked, this, &WindowDesign::pwm_up);
        /* PWM :: Row[3] */
        QPushButton *pwm_downButton = new QPushButton("DOWN", this);
        pwm_downButton->setGeometry(dev.xGap*3 + dev.xText + dev.xUnit, dev.yGap*14 + dev.yLogo*3 + dev.yUnit*8, dev.xUnit, dev.yUnit);
        connect(pwm_downButton, &QPushButton::clicked, this, &WindowDesign::pwm_down);

        /* Horizontal Separator */
        QFrame *hLine3 = new QFrame(this);
        hLine3->setGeometry(dev.xGap, dev.yGap*15 + dev.yLogo*3 + dev.yUnit*9, dev.xGap*3 + dev.xText + dev.xUnit*2, dev.separatorWidth);
        hLine3->setFrameShape(QFrame::HLine);
        hLine3->setFrameShadow(QFrame::Sunken);
    }

private slots:

    void i2c_execute()
    {
        bool addressFlag, registerFlag;

        QString addressText = i2c_addressField->text();
        (void)addressText.toInt(&addressFlag, 16);

        QString registerText = i2c_registerField->text();
        (void)registerText.toInt(&registerFlag, 16);

        if (addressFlag && registerFlag)
        {
            qDebug() << "[ADD, REG] = [" << addressText << "," << registerText << "]";
        }
        else
        {
            if(!addressFlag) QMessageBox::warning(this, "Invalid Input", "Please enter a valid Device Address");
            else if(!registerFlag) QMessageBox::warning(this, "Invalid Input", "Please enter a valid Register Address");
        }
        printToConsole("[I2C] Execute");
    }

    void spi_execute()
    {
        printToConsole("[SPI] Execute");
    }

    void pwm_execute()
    {
        printToConsole("[PWM] Execute");
    }

    void pwm_up()
    {
        printToConsole("[PWM] Up");
    }

    void pwm_down()
    {
        printToConsole("[PWM] Down");
    }

    void printToConsole(const QString &message)
    {
        consoleOutput->appendPlainText(message);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WindowDesign window;
    window.show();
    return app.exec();
}
