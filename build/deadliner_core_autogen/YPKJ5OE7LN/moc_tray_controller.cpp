/****************************************************************************
** Meta object code from reading C++ file 'tray_controller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ui/tray_controller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tray_controller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
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
struct qt_meta_tag_ZN9deadliner2ui14TrayControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto deadliner::ui::TrayController::qt_create_metaobjectdata<qt_meta_tag_ZN9deadliner2ui14TrayControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "deadliner::ui::TrayController",
        "openMainWindowRequested",
        "",
        "openSectionRequested",
        "deadliner::ui::MainSection",
        "section",
        "startBreakRequested",
        "snoozeAllRequested",
        "minutes",
        "pauseRequested",
        "skipNextRequested",
        "quitRequested"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'openMainWindowRequested'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'openSectionRequested'
        QtMocHelpers::SignalData<void(deadliner::ui::MainSection)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Signal 'startBreakRequested'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'snoozeAllRequested'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Signal 'pauseRequested'
        QtMocHelpers::SignalData<void(int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Signal 'skipNextRequested'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'quitRequested'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TrayController, qt_meta_tag_ZN9deadliner2ui14TrayControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject deadliner::ui::TrayController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deadliner2ui14TrayControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deadliner2ui14TrayControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9deadliner2ui14TrayControllerE_t>.metaTypes,
    nullptr
} };

void deadliner::ui::TrayController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TrayController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->openMainWindowRequested(); break;
        case 1: _t->openSectionRequested((*reinterpret_cast<std::add_pointer_t<deadliner::ui::MainSection>>(_a[1]))); break;
        case 2: _t->startBreakRequested(); break;
        case 3: _t->snoozeAllRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->pauseRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->skipNextRequested(); break;
        case 6: _t->quitRequested(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< deadliner::ui::MainSection >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TrayController::*)()>(_a, &TrayController::openMainWindowRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayController::*)(deadliner::ui::MainSection )>(_a, &TrayController::openSectionRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayController::*)()>(_a, &TrayController::startBreakRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayController::*)(int )>(_a, &TrayController::snoozeAllRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayController::*)(int )>(_a, &TrayController::pauseRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayController::*)()>(_a, &TrayController::skipNextRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayController::*)()>(_a, &TrayController::quitRequested, 6))
            return;
    }
}

const QMetaObject *deadliner::ui::TrayController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *deadliner::ui::TrayController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deadliner2ui14TrayControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int deadliner::ui::TrayController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void deadliner::ui::TrayController::openMainWindowRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void deadliner::ui::TrayController::openSectionRequested(deadliner::ui::MainSection _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void deadliner::ui::TrayController::startBreakRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void deadliner::ui::TrayController::snoozeAllRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void deadliner::ui::TrayController::pauseRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void deadliner::ui::TrayController::skipNextRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void deadliner::ui::TrayController::quitRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
