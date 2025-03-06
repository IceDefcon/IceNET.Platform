#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QHBoxLayout>

class WindowDesign : public QWidget
{
public:
WindowDesign()
{
    setWindowTitle("IceNET Platform");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // I2C Device Address Row
    QHBoxLayout *addressRow = new QHBoxLayout();
    QLabel *addressLabel = new QLabel("I2C Device Address:", this);
    addressField = new QLineEdit(this);
    addressField->setPlaceholderText("7-bit Hex");

    QPushButton *exeButton = new QPushButton("EXE", this);
    exeButton->setFixedWidth(50); // Set button width
    connect(exeButton, &QPushButton::clicked, this, &WindowDesign::i2c_execute);

    // Set fixed size for label and input field
    addressLabel->setFixedSize(120, 25);
    addressField->setFixedSize(80, 25);

    // Set QSizePolicy for fixed behavior
    addressField->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    addressLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    addressRow->addWidget(addressLabel);
    addressRow->addWidget(addressField);
    addressRow->addWidget(exeButton);

    // Align addressRow to the left
    addressRow->setAlignment(Qt::AlignLeft);  // Align to the left

    mainLayout->addLayout(addressRow);

    // Register Address Row
    QHBoxLayout *registerRow = new QHBoxLayout();
    QLabel *registerLabel = new QLabel("Register Address:", this);
    registerField = new QLineEdit(this);
    registerField->setPlaceholderText("8-bit Hex");

    // Set fixed size for label and input field
    registerLabel->setFixedSize(120, 25);
    registerField->setFixedSize(80, 25);

    // Set QSizePolicy for fixed behavior
    registerField->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    registerLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    registerRow->addWidget(registerLabel);
    registerRow->addWidget(registerField);

    // Align registerRow to the left
    registerRow->setAlignment(Qt::AlignLeft);  // Align to the left

    mainLayout->addLayout(registerRow);

    // Set layout
    setLayout(mainLayout);
    adjustSize();
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
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WindowDesign window;
    window.show();
    return app.exec();
}
