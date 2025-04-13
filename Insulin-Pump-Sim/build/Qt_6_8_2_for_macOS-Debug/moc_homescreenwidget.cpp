/****************************************************************************
** Meta object code from reading C++ file 'homescreenwidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/views/homescreenwidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'homescreenwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16HomeScreenWidgetE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN16HomeScreenWidgetE = QtMocHelpers::stringData(
    "HomeScreenWidget",
    "updateStatus",
    "",
    "onCreateProfile",
    "onEditProfile",
    "onDeleteProfile",
    "onBolus",
    "onCharge",
    "toggleBasalDelivery",
    "startBasalDelivery",
    "updateProfileDisplay",
    "updateHistory",
    "updateGraph",
    "onCrashInsulin",
    "updateOptionsPage"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN16HomeScreenWidgetE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x0a,    1 /* Public */,
       3,    0,   93,    2, 0x0a,    2 /* Public */,
       4,    0,   94,    2, 0x0a,    3 /* Public */,
       5,    0,   95,    2, 0x0a,    4 /* Public */,
       6,    0,   96,    2, 0x0a,    5 /* Public */,
       7,    0,   97,    2, 0x0a,    6 /* Public */,
       8,    0,   98,    2, 0x0a,    7 /* Public */,
       9,    0,   99,    2, 0x0a,    8 /* Public */,
      10,    0,  100,    2, 0x0a,    9 /* Public */,
      11,    0,  101,    2, 0x0a,   10 /* Public */,
      12,    0,  102,    2, 0x0a,   11 /* Public */,
      13,    0,  103,    2, 0x0a,   12 /* Public */,
      14,    0,  104,    2, 0x0a,   13 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject HomeScreenWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN16HomeScreenWidgetE.offsetsAndSizes,
    qt_meta_data_ZN16HomeScreenWidgetE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN16HomeScreenWidgetE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<HomeScreenWidget, std::true_type>,
        // method 'updateStatus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCreateProfile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEditProfile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeleteProfile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBolus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCharge'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleBasalDelivery'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startBasalDelivery'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateProfileDisplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateHistory'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateGraph'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCrashInsulin'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateOptionsPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void HomeScreenWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<HomeScreenWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->updateStatus(); break;
        case 1: _t->onCreateProfile(); break;
        case 2: _t->onEditProfile(); break;
        case 3: _t->onDeleteProfile(); break;
        case 4: _t->onBolus(); break;
        case 5: _t->onCharge(); break;
        case 6: _t->toggleBasalDelivery(); break;
        case 7: _t->startBasalDelivery(); break;
        case 8: _t->updateProfileDisplay(); break;
        case 9: _t->updateHistory(); break;
        case 10: _t->updateGraph(); break;
        case 11: _t->onCrashInsulin(); break;
        case 12: _t->updateOptionsPage(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *HomeScreenWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HomeScreenWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN16HomeScreenWidgetE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int HomeScreenWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
