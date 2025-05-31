/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>
#include <QDebug>
#include <QLabel>

#include <iostream>
#include <cstdint>

#include "KernelCtrl.h"

typedef struct
{
    uint32_t xWindow;
    uint32_t yWindow;
    uint32_t xGap;
    uint32_t yGap;
    uint32_t xLogo;
    uint32_t yLogo;
    uint32_t xText;
    uint32_t xUnit;
    uint32_t yUnit;
    uint32_t separatorLength;
    uint32_t separatorWidth;
} mainWindowType;

typedef struct
{
    uint32_t xPosition;
    uint32_t yPosition;
    uint32_t xSize;
    uint32_t ySize;
} mainConsoleType;

class mainCtrl : public QWidget
{
    Q_OBJECT

    QPlainTextEdit *m_mainConsoleOutput;

    std::unique_ptr<KernelCtrl> m_instanceMainCtrl;

private slots:



public:

    mainCtrl();
    ~mainCtrl();

private slots:

    void setupWindow();
    void setupMainConsole();
    void printToMainConsole(const QString &message);
    void setupNetworkControl();
    void setupSeparators();

    void createMainCtrl();
};
