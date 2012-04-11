// Local
#include "CuteIPCSignalHandler_p.h"
#include "CuteIPCService.h"

// Qt
#include <QDebug>


CuteIPCSignalHandler::CuteIPCSignalHandler(const QString& signature, CuteIPCService* parent)
  : QObject(parent),
    m_signature(signature)
{
  qDebug() << Q_FUNC_INFO;
}


CuteIPCSignalHandler::~CuteIPCSignalHandler()
{
  qDebug() << Q_FUNC_INFO;
}


void CuteIPCSignalHandler::relaySlot(void** args)
{
  // TODO
}


void CuteIPCSignalHandler::addListener(CuteIPCServiceConnection* listener)
{
  // TODO
}


void CuteIPCSignalHandler::listenerDestroyed(QObject* listener)
{
  // TODO
}


// Our Meta Object
// modify carefully: this has been hand-edited!
// the relaySlot slot has local ID 1 (we use this when calling QMetaObject::connect)
// it also gets called with the void** array
static const uint qt_meta_data_CuteIPCSignalHandler[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   22,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      54,   21,   21,   21, 0x0a,
      75,   66,   21,   21, 0x0a,
     114,   66,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CuteIPCSignalHandler[] = {
    "CuteIPCSignalHandler\0\0data\0"
    "signalCaptured(QByteArray)\0relaySlot()\0"
    "listener\0addListener(CuteIPCServiceConnection*)\0"
    "listenerDestroyed(QObject*)\0"
};

void CuteIPCSignalHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CuteIPCSignalHandler *_t = static_cast<CuteIPCSignalHandler *>(_o);
        switch (_id) {
        case 0: _t->signalCaptured((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 1: _t->relaySlot(_a); break; // HAND EDIT: Added _a argument
        case 2: _t->addListener((*reinterpret_cast< CuteIPCServiceConnection*(*)>(_a[1]))); break;
        case 3: _t->listenerDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CuteIPCSignalHandler::staticMetaObjectExtraData = {
    0,  qt_static_metacall
};

const QMetaObject CuteIPCSignalHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CuteIPCSignalHandler,
      qt_meta_data_CuteIPCSignalHandler, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CuteIPCSignalHandler::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CuteIPCSignalHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CuteIPCSignalHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CuteIPCSignalHandler))
        return static_cast<void*>(const_cast< CuteIPCSignalHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int CuteIPCSignalHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void CuteIPCSignalHandler::signalCaptured(const QByteArray & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
