#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPainter>
#include <QWidget>
#include <QLabel>
#include <QDebug>

typedef struct
{
    unsigned int x0;
    unsigned int y0;
    unsigned int yGap;
    unsigned int ySize;
    unsigned int textSize;
    unsigned int separatorSize;
}dimentionsType;

static dimentionsType dim =
{
    .x0 = 10,
    .y0 = 10,
    .yGap = 30,
    .ySize = 25,
    .textSize = 120,
    .separatorSize = 2,
};

typedef enum
{
    ROW_0 = 0,
    ROW_1,
    ROW_2,
    ROW_3,
    ROW_4,
    ROW_5,
    ROW_6,
    ROW_7,
    ROW_8,
    ROW_9,
} rowType;

class WindowDesign : public QWidget
{
public:

WindowDesign()
{
    setWindowTitle("IceNET Platform");
    setFixedSize(800, 600); // Set fixed window size

    // Address Row
    QLabel *addressLabel = new QLabel("I2C Device Address", this);
    addressLabel->setGeometry(dim.x0, dim.y0 + dim.yGap*ROW_0, dim.textSize, dim.ySize);
    addressField = new QLineEdit(this);
    addressField->setGeometry(180, dim.y0 + dim.yGap*ROW_0, 80, dim.ySize);
    addressField->setPlaceholderText("7-bit Hex");
    addressField->setText("0x69");
    QPushButton *exeButton = new QPushButton("EXE", this);
    exeButton->setGeometry(270, dim.y0 + dim.yGap*ROW_0, 50, dim.ySize);
    connect(exeButton, &QPushButton::clicked, this, &WindowDesign::i2c_execute);

    // Register Row
    QLabel *registerLabel = new QLabel("Register Address", this);
    registerLabel->setGeometry(dim.x0, dim.y0 + dim.yGap*ROW_1, dim.textSize, dim.ySize);
    registerField = new QLineEdit(this);
    registerField->setGeometry(180, dim.y0 + dim.yGap, 80, dim.ySize);
    registerField->setPlaceholderText("8-bit Hex");
    registerField->setText("0x00");

    QLabel *dataLabel = new QLabel("Write Data", this);
    dataLabel->setGeometry(dim.x0, dim.y0 + dim.yGap*ROW_2, dim.textSize, dim.ySize);
    dataField = new QLineEdit(this);
    dataField->setGeometry(180, dim.y0 + dim.yGap*2, 80, dim.ySize);
    dataField->setPlaceholderText("8-bit Hex");
    dataField->setText("0x00");
    dataField->setDisabled(true);

    QCheckBox *writeEnableCheck = new QCheckBox("WR", this);
    writeEnableCheck->setGeometry(270, dim.y0 + dim.yGap*2, 100, dim.ySize);
    connect(writeEnableCheck, &QCheckBox::toggled, dataField, &QLineEdit::setEnabled);

    // Separator (Horizontal Line)
    QFrame *hLine1 = new QFrame(this);
    hLine1->setGeometry(dim.x0, 100, 320, dim.separatorSize);
    hLine1->setFrameShape(QFrame::HLine);
    hLine1->setFrameShadow(QFrame::Sunken);

    QFrame *hLine2 = new QFrame(this);
    hLine2->setGeometry(dim.x0, 200, 320, dim.separatorSize);
    hLine2->setFrameShape(QFrame::HLine);
    hLine2->setFrameShadow(QFrame::Sunken);

    QFrame *hLine3 = new QFrame(this);
    hLine3->setGeometry(dim.x0, 300, 320, dim.separatorSize);
    hLine3->setFrameShape(QFrame::HLine);
    hLine3->setFrameShadow(QFrame::Sunken);

    // Separator (Vertical Line between fields)
    QFrame *vLine1 = new QFrame(this);
    vLine1->setGeometry(330, 10, 2, 290);
    vLine1->setFrameShape(QFrame::VLine);
    vLine1->setFrameShadow(QFrame::Sunken);
}

private slots:
    void i2c_execute()
    {
        bool addressFlag, registerFlag;

        QString addressText = addressField->text();
        (void)addressText.toInt(&addressFlag, 16);

        QString registerText = registerField->text();
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
    }

private:
    QLineEdit *addressField;
    QLineEdit *registerField;
    QLineEdit *dataField;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WindowDesign window;
    window.show();
    return app.exec();
}
