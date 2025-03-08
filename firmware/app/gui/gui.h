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
#include <QTimer>
#include <QFont>

#include "DroneCtrl.h"
#include <thread>
#include <mutex>

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

    QPlainTextEdit *m_consoleOutput;

    QLineEdit *m_i2c_addressField;
    QLineEdit *m_i2c_registerField;
    QLineEdit *m_i2c_burstField;
    QLineEdit *m_i2c_dataField;

    QLineEdit *m_spi_addressField;
    QLineEdit *m_spi_registerField;
    QLineEdit *m_spi_burstField;
    QLineEdit *m_spi_dataField;

    QLineEdit *m_pwm_dataField;

    std::unique_ptr<DroneCtrl> m_instanceDroneCtrl;
    std::thread m_threadMain;
    std::mutex m_threadMutex;
    bool m_threadKill;

public:

    gui();
    ~gui();

    void setupWindow();
    void setupConsole();
    void setupI2C();
    void setupSPI();
    void setupPWM();
    void setupSeparators();
    void setupProcess();

private slots:

    void i2c_execute();
    void spi_execute();
    void pwm_execute();

    void pwm_up();
    void pwm_down();

    void printToConsole(const QString &message);

    void initThread();
    void shutdownThread();
    void threadMain();
};
