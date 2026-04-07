/****************************************************************************
** Meta object code from reading C++ file 'app_controller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/app/app_controller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'app_controller.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9deadliner3app13AppControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto deadliner::app::AppController::qt_create_metaobjectdata<qt_meta_tag_ZN9deadliner3app13AppControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "deadliner::app::AppController",
        "refreshState",
        "",
        "handleOccurrence",
        "domain::ReminderOccurrence",
        "occurrence",
        "showMainWindow",
        "showSection",
        "ui::MainSection",
        "section",
        "addEvent",
        "editEvent",
        "id",
        "deleteEvent",
        "addProfile",
        "editProfile",
        "deleteProfile",
        "saveSettings",
        "domain::AppSettings",
        "settings",
        "domain::QuietHoursPolicy",
        "policy",
        "startBreakNow",
        "snoozeAll",
        "minutes",
        "pauseReminders",
        "skipNextReminder",
        "quitApplication"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'refreshState'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleOccurrence'
        QtMocHelpers::SlotData<void(const domain::ReminderOccurrence &)>(3, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'showMainWindow'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showSection'
        QtMocHelpers::SlotData<void(ui::MainSection)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'addEvent'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'editEvent'
        QtMocHelpers::SlotData<void(qint64)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 12 },
        }}),
        // Slot 'deleteEvent'
        QtMocHelpers::SlotData<void(qint64)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 12 },
        }}),
        // Slot 'addProfile'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'editProfile'
        QtMocHelpers::SlotData<void(qint64)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 12 },
        }}),
        // Slot 'deleteProfile'
        QtMocHelpers::SlotData<void(qint64)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 12 },
        }}),
        // Slot 'saveSettings'
        QtMocHelpers::SlotData<void(domain::AppSettings, domain::QuietHoursPolicy)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 18, 19 }, { 0x80000000 | 20, 21 },
        }}),
        // Slot 'startBreakNow'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'snoozeAll'
        QtMocHelpers::SlotData<void(int)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'pauseReminders'
        QtMocHelpers::SlotData<void(int)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'skipNextReminder'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'quitApplication'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AppController, qt_meta_tag_ZN9deadliner3app13AppControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject deadliner::app::AppController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deadliner3app13AppControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deadliner3app13AppControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9deadliner3app13AppControllerE_t>.metaTypes,
    nullptr
} };

void deadliner::app::AppController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AppController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->refreshState(); break;
        case 1: _t->handleOccurrence((*reinterpret_cast<std::add_pointer_t<domain::ReminderOccurrence>>(_a[1]))); break;
        case 2: _t->showMainWindow(); break;
        case 3: _t->showSection((*reinterpret_cast<std::add_pointer_t<ui::MainSection>>(_a[1]))); break;
        case 4: _t->addEvent(); break;
        case 5: _t->editEvent((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 6: _t->deleteEvent((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 7: _t->addProfile(); break;
        case 8: _t->editProfile((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 9: _t->deleteProfile((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 10: _t->saveSettings((*reinterpret_cast<std::add_pointer_t<domain::AppSettings>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<domain::QuietHoursPolicy>>(_a[2]))); break;
        case 11: _t->startBreakNow(); break;
        case 12: _t->snoozeAll((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->pauseReminders((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->skipNextReminder(); break;
        case 15: _t->quitApplication(); break;
        default: ;
        }
    }
}

const QMetaObject *deadliner::app::AppController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *deadliner::app::AppController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deadliner3app13AppControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int deadliner::app::AppController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
