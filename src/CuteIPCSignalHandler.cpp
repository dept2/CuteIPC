// Local
#include "CuteIPCSignalHandler_p.h"
#include "CuteIPCService.h"
#include "CuteIPCServiceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QDebug>
#include <QObject>
#include <QMetaMethod>


CuteIPCSignalHandler::CuteIPCSignalHandler(const QString& signature, QObject* parent)
  : QObject(parent),
    m_signature(signature)
{
  qDebug() << Q_FUNC_INFO;

  QMetaObject::connect(this,
                       this->metaObject()->indexOfSignal("destroyed(QString)"),
                       parent,
                       parent->metaObject()->indexOfSlot(
                          QMetaObject::normalizedSignature("_q_removeSignalHandler(QString)"))
                       );
}


void CuteIPCSignalHandler::setSignalParametersInfo(QObject *owner, const QString& signature)
{
  QMetaMethod method = owner->metaObject()->method(
        owner->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(signature.toAscii())));
  m_signalParametersInfo = method.parameterTypes();
}


CuteIPCSignalHandler::~CuteIPCSignalHandler()
{
  qDebug() << Q_FUNC_INFO << ":" << this;
}


void CuteIPCSignalHandler::relaySlot(void** args)
{
  qDebug() << Q_FUNC_INFO;

  if (m_signalParametersInfo.isEmpty())
    setSignalParametersInfo(parent(), m_signature);

  CuteIPCMessage::Arguments messageArguments;

  for (int i = 0; i < m_signalParametersInfo.size(); ++i)
  {
    const QByteArray& type = m_signalParametersInfo[i];

    //call arguments are started from index 1
    messageArguments.push_back(QGenericArgument(qstrdup(QString(type).toLatin1()),args[i+1]));
  }

  CuteIPCMessage message(CuteIPCMessage::MessageSignal, m_signature, messageArguments);
  QByteArray serializedMessage = CuteIPCMarshaller::marshallMessage(message);

  //cleanup memory
  foreach (const QGenericArgument& arg, messageArguments)
    delete[] arg.name();

  emit signalCaptured(serializedMessage);
}


void CuteIPCSignalHandler::addListener(CuteIPCServiceConnection* listener)
{
  m_listeners.push_back(listener);

  QMetaObject::connect(listener,
                       listener->metaObject()->indexOfSignal(
                          QMetaObject::normalizedSignature("destroyed(QObject*)")),
                       this,
                       this->metaObject()->indexOfSlot("listenerDestroyed(QObject*)")
                       );

  QMetaObject::connect(this,
                       this->metaObject()->indexOfSignal(
                           QMetaObject::normalizedSignature("signalCaptured(QByteArray)")),
                       listener,
                       listener->metaObject()->indexOfSlot(
                           QMetaObject::normalizedSignature("sendSignal(QByteArray)"))
                       );
}


void CuteIPCSignalHandler::listenerDestroyed(QObject* listener)
{
  for (int i = 0; i < m_listeners.size(); ++i)
  {
    if (qobject_cast<QObject*>(m_listeners[i])== listener)
    {
      m_listeners.removeAt(i);
      qDebug() << "Remove listener:" << listener;
      break;
    }
  }

  if (m_listeners.length() == 0)
  {
    emit destroyed(m_signature);
    this->deleteLater();
  }
}


// Our Meta Object
// modify carefully: this has been hand-edited!
// the relaySlot slot has local ID 1 (we use this when calling QMetaObject::connect)
// it also gets called with the void** array

static const uint qt_meta_data_CuteIPCSignalHandler[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   22,   21,   21, 0x05,
      64,   54,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      83,   21,   21,   21, 0x0a,
     104,   95,   21,   21, 0x0a,
     143,   95,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CuteIPCSignalHandler[] = {
    "CuteIPCSignalHandler\0\0data\0"
    "signalCaptured(QByteArray)\0signature\0"
    "destroyed(QString)\0relaySlot()\0listener\0"
    "addListener(CuteIPCServiceConnection*)\0"
    "listenerDestroyed(QObject*)\0"
};

const QMetaObject CuteIPCSignalHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CuteIPCSignalHandler,
      qt_meta_data_CuteIPCSignalHandler, 0 }
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
        switch (_id) {
        case 0: signalCaptured((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 1: destroyed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: relaySlot(_a); break;
        case 3: addListener((*reinterpret_cast< CuteIPCServiceConnection*(*)>(_a[1]))); break;
        case 4: listenerDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CuteIPCSignalHandler::signalCaptured(const QByteArray & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CuteIPCSignalHandler::destroyed(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
