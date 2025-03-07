/****************************************************************************
** Meta object code from reading C++ file 'gui.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gui/gui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_gui_t {
    QByteArrayData data[9];
    char stringdata0[80];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_gui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_gui_t qt_meta_stringdata_gui = {
    {
QT_MOC_LITERAL(0, 0, 3), // "gui"
QT_MOC_LITERAL(1, 4, 11), // "i2c_execute"
QT_MOC_LITERAL(2, 16, 0), // ""
QT_MOC_LITERAL(3, 17, 11), // "spi_execute"
QT_MOC_LITERAL(4, 29, 11), // "pwm_execute"
QT_MOC_LITERAL(5, 41, 6), // "pwm_up"
QT_MOC_LITERAL(6, 48, 8), // "pwm_down"
QT_MOC_LITERAL(7, 57, 14), // "printToConsole"
QT_MOC_LITERAL(8, 72, 7) // "message"

    },
    "gui\0i2c_execute\0\0spi_execute\0pwm_execute\0"
    "pwm_up\0pwm_down\0printToConsole\0message"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_gui[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x08 /* Private */,
       3,    0,   45,    2, 0x08 /* Private */,
       4,    0,   46,    2, 0x08 /* Private */,
       5,    0,   47,    2, 0x08 /* Private */,
       6,    0,   48,    2, 0x08 /* Private */,
       7,    1,   49,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,

       0        // eod
};

void gui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        gui *_t = static_cast<gui *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->i2c_execute(); break;
        case 1: _t->spi_execute(); break;
        case 2: _t->pwm_execute(); break;
        case 3: _t->pwm_up(); break;
        case 4: _t->pwm_down(); break;
        case 5: _t->printToConsole((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject gui::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_gui.data,
      qt_meta_data_gui,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *gui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *gui::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_gui.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int gui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
