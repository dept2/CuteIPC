// Local
#include "CuteIPCInterface.h"
#include "CuteIPCInterface_p.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCSignalHandler_p.h"

// Qt
#include <QLocalSocket>
#include <QMetaObject>
#include <QTime>
#include <QEventLoop>
#include <QMetaType>
#include <QTimer>


/*!
    \class CuteIPCInterface

    \brief The CuteIPCInterface class provides an IPC-client,
    intended for sending remote call requests and Qt signals
    to the server through the QLocalSocket connection.

    To connect to the server, call connectToServer() method.

    Use call() and callNoReply() methods to send method invoke requests to the server
    (which are synchronous and asynchronous respectively).
    The signature of these methods concurs with QMetaObject::invokeMethod() method signature.
    Thus, you can invoke remote methods the same way as you did it locally through the QMetaObject.

    You can also use a remoteConnect() to connect the remote signal to the slot
    of some local object.

    Contrarily, you can connect the local signal to the remote slot, by using
    remoteSlotConnect().

    \sa CuteIPCService
*/

CuteIPCInterfacePrivate::CuteIPCInterfacePrivate()
    : m_socket(0), m_connection(0)
{}


CuteIPCInterfacePrivate::~CuteIPCInterfacePrivate()
{}


void CuteIPCInterfacePrivate::registerSocket()
{
  Q_Q(CuteIPCInterface);
  m_socket = new QLocalSocket(q);
}


bool CuteIPCInterfacePrivate::checkConnectCorrection(const QString& signal, const QString& slot)
{
  if (signal[0] != '2' || slot[0] != '1')
    return false;

  QString signalSignature = signal.mid(1);
  QString slotSignature = slot.mid(1);

  if (!QMetaObject::checkConnectArgs(signalSignature.toAscii(), slotSignature.toAscii()))
  {
    qDebug() << "ERROR: incompatible signatures" << signalSignature << slotSignature;
    return false;
  }
  return true;
}


bool CuteIPCInterfacePrivate::sendRemoteConnectionRequest(const QString &signal)
{
  qDebug() << "REMOTE ACTION: Requesting connection to signal" << signal;
  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, signal);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);
  bool ok = sendSynchronousRequest(request);
  return ok;
}


bool CuteIPCInterfacePrivate::sendSynchronousRequest(const QByteArray& request)
{
//  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);

  QEventLoop loop;
  QObject::connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
  loop.exec();

  qDebug() << "ACTION: call was finished. stop local event loop...";

  return m_connection->lastCallSuccessful();
}


void CuteIPCInterfacePrivate::registerConnection(const QString& signalSignature,
                                                 QObject *reciever,
                                                 const QString& slotSignature)
{
  Q_Q(CuteIPCInterface);
  m_connections.insert(signalSignature, MethodData(reciever, slotSignature));
  QObject::connect(reciever, SIGNAL(destroyed(QObject*)), q, SLOT(_q_removeRemoteConnectionsOfObject(QObject*)));
}


void CuteIPCInterfacePrivate::_q_removeRemoteConnectionsOfObject(QObject* destroyedObject)
{
  qDebug() << "ACTION remove remote signal connections of object:" << destroyedObject;

  QMutableHashIterator<QString, MethodData> i(m_connections);
  while (i.hasNext()) {
      i.next();
      MethodData data = i.value();
      if (data.first == destroyedObject)
        i.remove();
  }
}


void CuteIPCInterfacePrivate::_q_invokeRemoteSignal(const QString& signalSignature,
                                                    const CuteIPCMessage::Arguments& arguments)
{
  QList<MethodData> recieversData = m_connections.values(signalSignature);
  foreach (const MethodData& data, recieversData) {
    if (!data.first)
      return;

    qDebug() << "ACTION SIGNAL Trying to invoke slot";

    QString methodName = data.second;
    methodName = methodName.left(methodName.indexOf("("));

    CuteIPCMessage::Arguments args = arguments;
    while (args.size() < 10)
      args.append(QGenericArgument());

    bool successfulInvoke = QMetaObject::invokeMethod(data.first, methodName.toAscii(),
            args.at(0), args.at(1), args.at(2),
            args.at(3), args.at(4), args.at(5),
            args.at(6), args.at(7), args.at(8),
            args.at(9));

    //TODO: need to cleanup memory!
    qDebug() << "ACTION SIGNAL: invoke slot:" << successfulInvoke;
  }
}


void CuteIPCInterfacePrivate::handleLocalSignalRequest(QObject* localObject,
                                                       const QString& signalSignature,
                                                       const QString& slotSignature)
{
  Q_Q(CuteIPCInterface);
  qDebug() << Q_FUNC_INFO;

  MethodData data(localObject, signalSignature);

  CuteIPCSignalHandler* handler = m_localSignalHandlers.value(data);
  if (!handler)
  {
    //create a new signal handler
//    qDebug() << "Create a new local signal handler for the signature: " << signalSignature;
    handler = new CuteIPCSignalHandler(slotSignature, q);
    handler->setSignalParametersInfo(localObject, signalSignature);

    m_localSignalHandlers.insert(data, handler);

    QMetaObject::connect(localObject,
                         localObject->metaObject()->indexOfSignal("destroyed(QObject*)"),
                         q,
                         q->metaObject()->indexOfSlot(
                            QMetaObject::normalizedSignature("_q_removeSignalHandlersOfObject(QObject*)"))
                         );

    QMetaObject::connect(localObject,
                         localObject->metaObject()->indexOfSignal(
                             QMetaObject::normalizedSignature(signalSignature.toAscii())),
                         handler,
                             handler->metaObject()->indexOfSlot("relaySlot()"));

    QMetaObject::connect(handler,
                         handler->metaObject()->indexOfSignal(
                             QMetaObject::normalizedSignature("signalCaptured(QByteArray)")),
                         q,
                         q->metaObject()->indexOfSlot(
                             QMetaObject::normalizedSignature("_q_sendSignal(QByteArray)"))
                         );
  }
}


void CuteIPCInterfacePrivate::_q_removeSignalHandlersOfObject(QObject* destroyedObject)
{
  qDebug() << "ACTION remove signal handlers of object:" << destroyedObject;

  QMutableHashIterator<MethodData, CuteIPCSignalHandler*> i(m_localSignalHandlers);
  while (i.hasNext()) {
      i.next();
      MethodData data = i.key();
      if (data.first == destroyedObject)
        i.remove();
  }
}


void CuteIPCInterfacePrivate::_q_sendSignal(const QByteArray &request)
{
  qDebug() << "REMOTE ACTION: Send signal...";
  m_connection->sendCallRequest(request);
}


/*!
    Creates a new CuteIPCInterface object with the given \a parent.

    \sa connectToServer()
 */
CuteIPCInterface::CuteIPCInterface(QObject* parent)
  : QObject(parent),
    d_ptr(new CuteIPCInterfacePrivate())
{
  Q_D(CuteIPCInterface);
  d->q_ptr = this;
  d->registerSocket();
}


CuteIPCInterface::CuteIPCInterface(CuteIPCInterfacePrivate& dd, QObject* parent)
  : QObject(parent),
    d_ptr(&dd)
{
  Q_D(CuteIPCInterface);
  d->q_ptr = this;
  d->registerSocket();
}


/*!
    Destroyes the object.
 */
CuteIPCInterface::~CuteIPCInterface()
{}


/*!
    Attempts to make a connection to the server with given name.
 */
bool CuteIPCInterface::connectToServer(const QString& name)
{
  Q_D(CuteIPCInterface);
  d->m_socket->connectToServer(name);
  bool connected = d->m_socket->waitForConnected(5000);
  if (!connected)
    d->m_socket->disconnectFromServer();

  if (connected)
  {
    d->m_connection = new CuteIPCInterfaceConnection(d->m_socket, this);
    connect(d->m_connection, SIGNAL(invokeRemoteSignal(QString,CuteIPCMessage::Arguments)),
          this, SLOT(_q_invokeRemoteSignal(QString,CuteIPCMessage::Arguments)));
    qDebug() << "Connected:" << connected;
  }

  return connected;
}


/*!
    Disconnects from server by closing the socket.
 */
void CuteIPCInterface::disconnectFromServer()
{
  Q_D(CuteIPCInterface);
  d->m_socket->disconnectFromServer();
}


/*!
    The method is used to connect the remote signal (on the server-side) to the slot
    of some local object.
    It returns true on success. False otherwise (the slot doesn't exist,
    of signatures are incompatible, or if the server replies with an error).

    \note It is recommended to use this method the same way as you call QObject::connect() method
    (by using SIGNAL() and SLOT() macros).
    \par
    For example, to connect the remote \a exampleSignal() signal to the \a exampleSlot() of some local \a object,
    you can type:
    \code remoteConnect(SIGNAL(exampleSignal()), object, SLOT(exampleSlot())); \endcode

    \note This method doesn't establish the connection to the server, you must use connectToServer() before.

    \sa remoteSlotConnect()
 */
bool CuteIPCInterface::remoteConnect(const char* signal, QObject* object, const char* slot)
{
  Q_D(CuteIPCInterface);
  QString signalSignature = QString::fromAscii(signal);
  QString slotSignature = QString::fromAscii(slot);

  if (!d->checkConnectCorrection(signalSignature, slotSignature))
    return false;

  signalSignature = signalSignature.mid(1);
  slotSignature = slotSignature.mid(1);

  int slotIndex = object->metaObject()->indexOfSlot(
        QMetaObject::normalizedSignature(slotSignature.toAscii()));
  if (slotIndex == -1)
  {
    qDebug() << "ERROR: Slot doesn't exist:" + slotSignature << "object:" << object;
    return false;
  }

  if (!d->m_connections.contains(signalSignature))
  {
    if (!d->sendRemoteConnectionRequest(signalSignature))
      return false;
  }

  d->registerConnection(signalSignature, object, slotSignature);
  return true;
}


/*!
    The method is used to connect the signal of some local object (on the client-side) to the remote slot
    of the server.

    It returns true on success. False otherwise (If the local signal doesn't exist, or signatures are incompatible).

    \note It is recommended to use this method the same way as you call QObject::connect() method
    (by using SIGNAL() and SLOT() macros).
    \par
    For example, to connect the exampleSignal() signal of some local \a object to the remote \a exampleSlot() slot,
    you can type:
    \code remoteSlotConnect(object, SIGNAL(exampleSignal()), SLOT(exampleSlot())); \endcode

    \warning The method doesn't check the existance of the remote slot on the server-side.

    \sa remoteConnect(), call()
 */
bool CuteIPCInterface::remoteSlotConnect(QObject *localObject, const char *signal, const char *remoteSlot)
{
  Q_D(CuteIPCInterface);

  QString signalSignature = QString::fromAscii(signal);
  QString slotSignature = QString::fromAscii(remoteSlot);

  if (!d->checkConnectCorrection(signalSignature, slotSignature))
    return false;

  signalSignature = signalSignature.mid(1);
  slotSignature = slotSignature.mid(1);

  int signalIndex = localObject->metaObject()->indexOfSignal(
        QMetaObject::normalizedSignature(signalSignature.toAscii()));
  if (signalIndex == -1)
  {
    qDebug() << "ERROR: Signal doesn't exist:" + signalSignature << "object:" << localObject;
    return false;
  }

  d->handleLocalSignalRequest(localObject, signalSignature, slotSignature);
  return true;
}


/*!
    Invokes the remote \a method (of the server). Returns true if the invokation was successful, false otherwise.
    The invokation is synchronous (which means that client will be waiting for the response).
    See callNoReply() method for asynchronous invokation.

    The signature of this method is completely concurs with QMetaObject::invokeMethod() Qt method signature.
    Thus, you can use it the same way as you did it locally, with invokeMethod().

    The return value of the member function call is placed in \a ret.
    You can pass up to ten arguments (val0, val1, val2, val3, val4, val5, val6, val7, val8, and val9) to the member function.

    \note To set arguments, you must enclose them using Q_ARG and Q_RETURN_ARG macros.
    \note This method doesn't establish the connection to the server, you must use connectToServer() first.
    \sa callNoReply()
 */
bool CuteIPCInterface::call(const QString& method, QGenericReturnArgument ret, QGenericArgument val0,
                            QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4,
                            QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8,
                            QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
//  qDebug() << "";
//  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9, QString::fromLatin1(ret.name()));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  d->m_connection->setReturnedObject(ret);
  qDebug() << "REMOTE ACTION: Trying to call" << method;

  return d->sendSynchronousRequest(request);
}


/*!
    This function overloads call() method.
    This overload can be used if the return value of the member is of no interest.

    \note To set arguments, you must enclose them using Q_ARG macro.
    \note This method doesn't establish the connection to the server, you must use connectToServer() before.
    \sa callNoReply()
 */
bool CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
//  qDebug() << "";
//  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "REMOTE ACTION: Trying to call" << method;

  return d->sendSynchronousRequest(request);
}



/*!
    Invokes the remote \a method (of the server). Returns true if the invokation was successful, false otherwise.
    Unlike the process of call() method, the invokation is asynchronous
    (which means that the client will not waiting for the response).

    The signature of this method is completely concurs with QMetaObject::invokeMethod() Qt method signature
    (without return value).
    Thus, you can use it the same way as you did it locally, with invokeMethod().

    You can pass up to ten arguments (val0, val1, val2, val3, val4, val5, val6, val7, val8, and val9) to the member function.

    \note To set arguments, you must enclose them using Q_ARG macro.
    \note This method doesn't establish the connection to the server, you must use connectToServer() before.
    \sa call(), connectToServer()
 */
void CuteIPCInterface::callNoReply(const QString& method, QGenericArgument val0, QGenericArgument val1,
                                        QGenericArgument val2, QGenericArgument val3, QGenericArgument val4,
                                        QGenericArgument val5, QGenericArgument val6, QGenericArgument val7,
                                        QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
//  qDebug() << "";
//  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithoutReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "REMOTE ACTION: Call (asynchronously)" << method;
//  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  d->m_connection->sendCallRequest(request);
}


/*!
    Returns the error that last occured.
 */
QString CuteIPCInterface::lastError() const
{
  Q_D(const CuteIPCInterface);
  return d->m_connection->lastError();
}
