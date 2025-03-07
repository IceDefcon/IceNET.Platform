#include <QPlainTextEdit>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPainter>
#include <QWidget>
#include <QObject>
#include <QThread>
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

class gui : public QWidget
{
    Q_OBJECT

    QPlainTextEdit *consoleOutput;
    QLineEdit *i2c_addressField, *i2c_registerField, *i2c_burstField, *i2c_dataField;
    QLineEdit *spi_addressField, *spi_registerField, *spi_burstField, *spi_dataField;
    QLineEdit *pwm_dataField;

public:

    gui();

private slots:

    void i2c_execute();
    void spi_execute();
    void pwm_execute();
    void pwm_up();
    void pwm_down();
    void printToConsole(const QString &message);

};
