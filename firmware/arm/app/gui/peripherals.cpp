/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include "gui.h"

void gui::setupI2C()
{
    const mainWindowType* w = getMainWindow();

    /* I2C :: Row[0] */
    QLabel *i2c_label = new QLabel("I2C", this);
    QFont i2c_labelFont;
    i2c_labelFont.setPointSize(30);
    i2c_labelFont.setItalic(true);
    i2c_labelFont.setBold(true);
    i2c_label->setFont(i2c_labelFont);
    i2c_label->setGeometry(w->xGap , w->yGap, w->xLogo, w->yLogo);
    /* I2C :: Row[1] */
    QLabel *i2c_addressLabel = new QLabel("Device Address", this);
    i2c_addressLabel->setGeometry(w->xGap, w->yGap*2 + w->yLogo, w->xText, w->yUnit);
    m_i2c_addressField = new QLineEdit(this);
    m_i2c_addressField->setGeometry(w->xGap*2 + w->xText, w->yGap*2 + w->yLogo, w->xUnit, w->yUnit);
    m_i2c_addressField->setText("0x53");
    QPushButton *i2c_exeButton = new QPushButton("EXE", this);
    i2c_exeButton->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*2 + w->yLogo, w->xUnit, w->yUnit);
    connect(i2c_exeButton, &QPushButton::clicked, this, &gui::i2c_execute);
    /* I2C :: Row[2] */
    QLabel *i2c_registerLabel = new QLabel("Register Address", this);
    i2c_registerLabel->setGeometry(w->xGap, w->yGap*3 + w->yLogo + w->yUnit, w->xText, w->yUnit);
    m_i2c_registerField = new QLineEdit(this);
    m_i2c_registerField->setGeometry(w->xGap*2 + w->xText, w->yGap*3 + w->yLogo + w->yUnit, w->xUnit, w->yUnit);
    m_i2c_registerField->setText("0x00");
    m_i2c_burstField = new QLineEdit(this);
    m_i2c_burstField->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*3 + w->yLogo + w->yUnit, w->xUnit, w->yUnit);
    m_i2c_burstField->setText("0x01");
    /* I2C :: Row[3] */
    QLabel *i2c_dataLabel = new QLabel("Write Data", this);
    i2c_dataLabel->setGeometry(w->xGap, w->yGap*4 + w->yLogo + w->yUnit*2, w->xText, w->yUnit);
    m_i2c_dataField = new QLineEdit(this);
    m_i2c_dataField->setGeometry(w->xGap*2 + w->xText , w->yGap*4 + w->yLogo + w->yUnit*2, w->xUnit, w->yUnit);
    m_i2c_dataField->setText("0x00");
    m_i2c_dataField->setDisabled(true);
    m_i2c_writeTick = new QCheckBox("WR", this);
    m_i2c_writeTick->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*4 + w->yLogo + w->yUnit*2, w->xUnit, w->yUnit);
    connect(m_i2c_writeTick, &QCheckBox::toggled, m_i2c_dataField, &QLineEdit::setEnabled);
}

void gui::setupSPI()
{
    const mainWindowType* w = getMainWindow();

    /* SPI :: Row[0] */
    QLabel *spi_label = new QLabel("SPI", this);
    QFont spi_labelFont;
    spi_labelFont.setPointSize(30);
    spi_labelFont.setItalic(true);
    spi_labelFont.setBold(true);
    spi_label->setFont(spi_labelFont);
    spi_label->setGeometry(w->xGap, w->yGap*6 + w->yLogo + w->yUnit*3, w->xLogo, w->yLogo);
    /* SPI :: Row[1] */
    QLabel *spi_addressLabel = new QLabel("FPGA Device ID", this);
    spi_addressLabel->setGeometry(w->xGap, w->yGap*7 + w->yLogo*2 + w->yUnit*3, w->xText, w->yUnit);
    m_spi_addressField = new QLineEdit(this);
    m_spi_addressField->setGeometry(w->xGap*2 + w->xText, w->yGap*7 + w->yLogo*2 + w->yUnit*3, w->xUnit, w->yUnit);
    m_spi_addressField->setText("0x11");
    QPushButton *spi_exeButton = new QPushButton("EXE", this);
    spi_exeButton->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*7 + w->yLogo*2 + w->yUnit*3, w->xUnit, w->yUnit);
    connect(spi_exeButton, &QPushButton::clicked, this, &gui::spi_execute);
    /* SPI :: Row[2] */
    QLabel *spi_registerLabel = new QLabel("Register Address", this);
    spi_registerLabel->setGeometry(w->xGap, w->yGap*8 + w->yLogo*2 + w->yUnit*4, w->xText, w->yUnit);
    m_spi_registerField = new QLineEdit(this);
    m_spi_registerField->setGeometry(w->xGap*2 + w->xText, w->yGap*8 + w->yLogo*2 + w->yUnit*4, w->xUnit, w->yUnit);
    m_spi_registerField->setText("0x92");
    m_spi_burstField = new QLineEdit(this);
    m_spi_burstField->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*8 + w->yLogo*2 + w->yUnit*4, w->xUnit, w->yUnit);
    m_spi_burstField->setText("0x01");
    /* SPI :: Row[3] */
    QLabel *spi_dataLabel = new QLabel("Write Data", this);
    spi_dataLabel->setGeometry(w->xGap, w->yGap*9 + w->yLogo*2 + w->yUnit*5, w->xText, w->yUnit);
    m_spi_dataField = new QLineEdit(this);
    m_spi_dataField->setGeometry(w->xGap*2 + w->xText, w->yGap*9 + w->yLogo*2 + w->yUnit*5, w->xUnit, w->yUnit);
    m_spi_dataField->setText("0x00");
    m_spi_dataField->setDisabled(true);
    m_spi_writeTick = new QCheckBox("WR", this);
    m_spi_writeTick->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*9 + w->yLogo*2 + w->yUnit*5, w->xUnit, w->yUnit);
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
    const mainWindowType* w = getMainWindow();

    /* PWM :: Row[0] */
    QLabel *pwm_label = new QLabel("PWM", this);
    QFont pwm_labelFont;
    pwm_labelFont.setPointSize(30);
    pwm_labelFont.setItalic(true);
    pwm_labelFont.setBold(true);
    pwm_label->setFont(pwm_labelFont);
    pwm_label->setGeometry(w->xGap, w->yGap*11 + w->yLogo*2 + w->yUnit*6, w->xLogo, w->yLogo);
    /* PWM :: Row[1] */
    QLabel *pwm_speedLabel = new QLabel("Speed [Hex]", this);
    pwm_speedLabel->setGeometry(w->xGap, w->yGap*12 + w->yLogo*3 + w->yUnit*6, w->xText, w->yUnit);
    m_pwm_dataField = new QLineEdit(this);
    m_pwm_dataField->setGeometry(w->xGap*2 + w->xText, w->yGap*12 + w->yLogo*3 + w->yUnit*6, w->xUnit, w->yUnit);
    m_pwm_dataField->setText("0x00");
    QPushButton *pwm_exeButton = new QPushButton("EXE", this);
    pwm_exeButton->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*12 + w->yLogo*3 + w->yUnit*6, w->xUnit, w->yUnit);

    auto pwmExecute = [this]()
    {
        pwm_execute(PWM_EXE);
    };
    connect(pwm_exeButton, &QPushButton::clicked, this, pwmExecute);

    /* PWM :: Row[2] */
    QPushButton *pwm_upButton = new QPushButton("UP", this);
    pwm_upButton->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*13 + w->yLogo*3 + w->yUnit*7, w->xUnit, w->yUnit);

    auto pwmUp = [this]()
    {
        pwm_execute(PWM_UP);
    };
    connect(pwm_upButton, &QPushButton::clicked, this, pwmUp);

    /* PWM :: Row[3] */
    QPushButton *pwm_downButton = new QPushButton("DOWN", this);
    pwm_downButton->setGeometry(w->xGap*3 + w->xText + w->xUnit, w->yGap*14 + w->yLogo*3 + w->yUnit*8, w->xUnit, w->yUnit);

    auto pwmDown = [this]()
    {
        pwm_execute(PWM_DOWN);
    };
    connect(pwm_downButton, &QPushButton::clicked, this, pwmDown);
}

void gui::setupEXT()
{
    const mainWindowType* w = getMainWindow();
    
    /* Row[0] */
    QLabel *spi_label = new QLabel("SPI.EXT", this);
    QFont spi_labelFont;
    spi_labelFont.setPointSize(30);
    spi_labelFont.setItalic(true);
    spi_labelFont.setBold(true);
    spi_label->setFont(spi_labelFont);
    spi_label->setGeometry(800 + w->xGap, w->yGap*6 + w->yUnit*3 + w->yLogo, w->xLogo, w->yLogo);

    /* Row[1] */
    QLabel *byte_0_Label = new QLabel("R/W -> Byte.[0]", this);
    byte_0_Label->setGeometry(800 + w->xGap, w->yGap*7 + w->yLogo*2 + w->yUnit*3, w->xText, w->yUnit);
    m_ext_Byte_0 = new QLineEdit(this);
    m_ext_Byte_0->setGeometry(800 + w->xGap*2 + w->xText, w->yGap*7 + w->yLogo*2 + w->yUnit*3, w->xUnit, w->yUnit);
    m_ext_Byte_0->setText("0x40");
    QPushButton *ext_exeButton = new QPushButton("EXE", this);
    ext_exeButton->setGeometry(800 + w->xGap*3 + w->xText + w->xUnit, w->yGap*7 + w->yLogo*2 + w->yUnit*3, w->xUnit, w->yUnit);
    connect(ext_exeButton, &QPushButton::clicked, this, &gui::ext_execute);
    /* Row[2] */
    QLabel *spi_registerLabel = new QLabel("R/W -> Byte.[1]", this);
    spi_registerLabel->setGeometry(800 + w->xGap, w->yGap*8 + w->yLogo*2 + w->yUnit*4, w->xText, w->yUnit);
    m_ext_Byte_1 = new QLineEdit(this);
    m_ext_Byte_1->setGeometry(800 + w->xGap*2 + w->xText, w->yGap*8 + w->yLogo*2 + w->yUnit*4, w->xUnit, w->yUnit);
    m_ext_Byte_1->setText("0x37");
    /* Row[3] */
    QLabel *spi_dataLabel = new QLabel("R/W -> Byte.[2]", this);
    spi_dataLabel->setGeometry(800 + w->xGap, w->yGap*9 + w->yLogo*2 + w->yUnit*5, w->xText, w->yUnit);
    m_ext_Byte_2 = new QLineEdit(this);
    m_ext_Byte_2->setGeometry(800 + w->xGap*2 + w->xText, w->yGap*9 + w->yLogo*2 + w->yUnit*5, w->xUnit, w->yUnit);
    m_ext_Byte_2->setText("0x00");
    m_ext_Byte_2->setDisabled(true);
    m_ext_writeTick = new QCheckBox("WR", this);
    m_ext_writeTick->setGeometry(800 + w->xGap*3 + w->xText + w->xUnit, w->yGap*9 + w->yLogo*2 + w->yUnit*5, w->xUnit, w->yUnit);
    connect(m_ext_writeTick, &QCheckBox::toggled, m_ext_Byte_2, &QLineEdit::setEnabled);
}
