// Local
#include "CuteIPCInterface.h"
#include "CuteIPCInterface_p.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCSignalHandler_p.h"
#include "CuteIPCInterfaceWorker.h"

// Qt
#include <QThread>
#include <QEventLoop>


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

    You can also use a remoteConnect() to connect the remote signal to the slot or signal
    of some local object.

    Contrarily, you can connect the local signal to the remote slot, by using
    remoteSlotConnect().

    \sa CuteIPCService
*/

CuteIPCInterfacePrivate::CuteIPCInterfacePrivate()
  : m_workerThread(new QThread),
    m_worker(new CuteIPCInterfaceWorker)
{
  m_worker->moveToThread(m_workerThread);
  m_workerThread->start();
}


CuteIPCInterfacePrivate::~CuteIPCInterfacePrivate()
{
  m_workerThread->quit();
  m_workerThread->wait();

  delete m_worker;
  delete m_workerThread;
}


void CuteIPCInterfacePrivate::registerSocket()
{
  QEventLoop loop;
  QObject::connect(m_worker, SIGNAL(registerSocketFinished()), &loop, SLOT(quit()));
  QMetaObject::invokeMethod(m_worker, "registerSocket");
  loop.exec();
}


bool CuteIPCInterfacePrivate::checkConnectCorrection(const QString& signal, const QString& method)
{
  if (signal[0] != '2' || (method[0] != '1' && method[0] != '2'))
    return false;

  QString signalSignature = signal.mid(1);
  QString methodSignature = method.mid(1);

  if (!QMetaObject::checkConnectArgs(signalSignature.toAscii(), methodSignature.toAscii()))
  {
    qWarning() << "CuteIPC:" << "Error: incompatible signatures" << signalSignature << methodSignature;
    m_lastError = "Incompatible signatures: " + signalSignature + "," + methodSignature;
    return false;
  }
  return true;
}


bool CuteIPCInterfacePrivate::checkRemoteSlotExistance(const QString& slot)
{
  DEBUG << "Check remote slot existance" << slot;
  CuteIPCMessage message(CuteIPCMessage::SlotConnectionRequest, slot);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);
  return sendSynchronousRequest(request);
}


bool CuteIPCInterfacePrivate::sendSynchronousRequest(const QByteArray& request, QGenericReturnArgument returnedObject)
{
  Q_Q(CuteIPCInterface);
  QLocalSocket socket;
  socket.connectToServer(m_serverName);
  bool connected = socket.waitForConnected(5000);
  if (!connected)
  {
    socket.disconnectFromServer();
    QString error("CuteIPC: Не удалось подключиться к серверу при вызове синхронного метода");
    qWarning() << error;
    _q_setLastError(error);
    return false;
  }

  CuteIPCInterfaceConnection connection(&socket);
  QObject::connect(&connection, SIGNAL(errorOccured(QString)), q, SLOT(_q_setLastError(QString)));

  connection.setReturnedObject(returnedObject);

  QEventLoop loop;
  QObject::connect(&connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
  QObject::connect(&connection, SIGNAL(socketDisconnected()), &loop, SLOT(quit()));
  connection.sendCallRequest(request);
  loop.exec();

  return connection.lastCallSuccessful();
}


void CuteIPCInterfacePrivate::_q_setLastError(QString lastError)
{
  this->m_lastError = lastError;
}


void CuteIPCInterfacePrivate::handleLocalSignalRequest(QObject* localObject, const QString& signalSignature,
                                                       const QString& slotSignature)
{
  Q_Q(CuteIPCInterface);

  MethodData data(localObject, signalSignature);

  QList<CuteIPCSignalHandler*> handlers = m_localSignalHandlers.values(data);
  CuteIPCSignalHandler* handler = 0;
  foreach (CuteIPCSignalHandler* existingHandler, handlers)
  {
    if (existingHandler->signature() == slotSignature)
      handler = existingHandler;
  }

  if (!handler)
  {
    handler = new CuteIPCSignalHandler(slotSignature, q);
    handler->setSignalParametersInfo(localObject, signalSignature);

    m_localSignalHandlers.insert(data, handler);

    QMetaObject::connect(localObject,
        localObject->metaObject()->indexOfSignal("destroyed(QObject*)"),
        q, q->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("_q_removeSignalHandlersOfObject(QObject*)")));

    QMetaObject::connect(localObject,
        localObject->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toAscii())),
        handler, handler->metaObject()->indexOfSlot("relaySlot()"));

    QMetaObject::connect(
        handler, handler->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("signalCaptured(QByteArray)")),
        q, q->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("_q_sendAsynchronousRequest(QByteArray)")));
  }
}


void CuteIPCInterfacePrivate::_q_removeSignalHandlersOfObject(QObject* destroyedObject)
{
  QMutableHashIterator<MethodData, CuteIPCSignalHandler*> i(m_localSignalHandlers);
  while (i.hasNext())
  {
    i.next();
    MethodData data = i.key();
    if (data.first == destroyedObject)
      i.remove();
  }
}


void CuteIPCInterfacePrivate::_q_sendAsynchronousRequest(const QByteArray& request)
{
  QMetaObject::invokeMethod(m_worker, "sendCallRequest", Q_ARG(QByteArray, request));
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

  QObject::connect(d->m_worker, SIGNAL(setLastError(QString)), this, SLOT(_q_setLastError(QString)));

  qRegisterMetaType<QGenericReturnArgument>("QGenericReturnArgument");
  qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
  qRegisterMetaType<CuteIPCMessage::Arguments>("CuteIPCMessage::Arguments");

  d->registerSocket();

}


CuteIPCInterface::CuteIPCInterface(CuteIPCInterfacePrivate& dd, QObject* parent)
  : QObject(parent),
    d_ptr(&dd)
{
  Q_D(CuteIPCInterface);
  d->q_ptr = this;

  QObject::connect(d->m_worker, SIGNAL(setLastError(QString)), this, SLOT(_q_setLastError(QString)));
  qRegisterMetaType<QGenericReturnArgument>("QGenericReturnArgument");
  qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
  qRegisterMetaType<CuteIPCMessage::Arguments>("CuteIPCMessage::Arguments");

  d->registerSocket();
}


/*!
    Destroyes the object.
 */
CuteIPCInterface::~CuteIPCInterface()
{
  delete d_ptr;
}


/*!
    Attempts to make a connection to the server with given name.
 */
bool CuteIPCInterface::connectToServer(const QString& name)
{
  Q_D(CuteIPCInterface);
  bool connected;

  QEventLoop loop;
  connect(d->m_worker, SIGNAL(connectToServerFinished()), &loop, SLOT(quit()));
  QMetaObject::invokeMethod(d->m_worker, "connectToServer", Q_ARG(QString, name), Q_ARG(void*, &connected));
  loop.exec();

  d->m_serverName = name;
  return connected;
}


/*!
    Disconnects from server by closing the socket.
 */
void CuteIPCInterface::disconnectFromServer()
{
  Q_D(CuteIPCInterface);
  QEventLoop loop;
  connect(d->m_worker, SIGNAL(disconnectFromServerFinished()), &loop, SLOT(quit()));
  QMetaObject::invokeMethod(d->m_worker, "disconnectFromServer");
  loop.exec();
}


/*!
    The method is used to connect the remote signal (on the server-side) to the slot or signal
    of some local object.
    It returns true on success. False otherwise (the slot doesn't exist,
    of signatures are incompatible, or if the server replies with an error).

    \note It is recommended to use this method the same way as you call QObject::connect() method
    (by using SIGNAL() and SLOT() macros).
    \par
    For example, to connect the remote \a exampleSignal() signal to the \a exampleSlot() of some local \a object,
    you can type:
    \code remoteConnect(SIGNAL(exampleSignal()), object, SLOT(exampleSlot())); \endcode

    \note This method doesn't establish the connection to the server, you must use connectToServer() first.

    \sa remoteSlotConnect()
 */
bool CuteIPCInterface::remoteConnect(const char* signal, QObject* object, const char* method)
{
  Q_D(CuteIPCInterface);
  QString signalSignature = QString::fromAscii(signal);
  QString methodSignature = QString::fromAscii(method);

  if (!d->checkConnectCorrection(signalSignature, methodSignature))
    return false;

  signalSignature = signalSignature.mid(1);
  methodSignature = methodSignature.mid(1);


  int methodIndex = -1;
  if (method[0] == '1')
    methodIndex = object->metaObject()->indexOfSlot(QMetaObject::normalizedSignature(methodSignature.toAscii()));
  else if (method[0] == '2')
    methodIndex = object->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(methodSignature.toAscii()));

  if (methodIndex == -1)
  {
    d->m_lastError = "Method (slot or signal) doesn't exist:" + methodSignature;
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << object;
    return false;
  }

  QMetaObject::invokeMethod(d->m_worker, "remoteConnect", Q_ARG(QString, signalSignature), Q_ARG(void*, object),
                            Q_ARG(QString, methodSignature));
  return true;
}

/*!
    Disconnects remote signal of server
    from local slot in object receiver.
    Returns true if the connection is successfully broken;
    otherwise returns false.

    \sa remoteConnect
 */
bool CuteIPCInterface::disconnectSignal(const char* signal, QObject* object, const char* method)
{
  Q_D(CuteIPCInterface);

  if (signal[0] != '2' || (method[0] != '1' && method[0] != '2'))
    return false;

  QString signalSignature = QString::fromAscii(signal).mid(1);
  QString methodSignature = QString::fromAscii(method).mid(1);

  QMetaObject::invokeMethod(d->m_worker, "disconnectSignal", Q_ARG(QString, signalSignature), Q_ARG(void*, object),
                            Q_ARG(QString, methodSignature));
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
bool CuteIPCInterface::remoteSlotConnect(QObject* localObject, const char* signal, const char* remoteSlot)
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
    d->m_lastError = "Signal doesn't exist:" + signalSignature;
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << localObject;
    return false;
  }

  if (!d->checkRemoteSlotExistance(slotSignature))
  {
    d->m_lastError = "Remote slot doesn't exist:" + slotSignature;
    return false;
  }

  d->handleLocalSignalRequest(localObject, signalSignature, slotSignature);
  return true;
}


/*!
    Disconnects local signal from remote slot of server.
    Returns true if the connection is successfully broken;
    otherwise returns false.

    \sa remoteSlotConnect
 */
bool CuteIPCInterface::disconnectSlot(QObject* localObject, const char* signal, const char* remoteSlot)
{
  Q_D(CuteIPCInterface);

  if (signal[0] != '2' || remoteSlot[0] != '1')
    return false;

  QString signalSignature = QString::fromAscii(signal).mid(1);
  QString slotSignature = QString::fromAscii(remoteSlot).mid(1);

  CuteIPCInterfacePrivate::MethodData data(localObject, signalSignature);

  QList<CuteIPCSignalHandler*> handlers = d->m_localSignalHandlers.values(data);
  foreach (CuteIPCSignalHandler* handler, handlers)
  {
    if (handler->signature() == slotSignature)
    {
      delete handler;
      d->m_localSignalHandlers.remove(data, handler);
    }
  }
  return true;
}


/*!
    Invokes the remote \a method (of the server). Returns true if the invokation was successful, false otherwise.
    The invokation is synchronous (which means that client will be waiting for the response).
    See callNoReply() method for asynchronous invokation.

    The signature of this method is completely concurs with QMetaObject::invokeMethod() Qt method signature.
    Thus, you can use it the same way as you did it locally, with invokeMethod().

    The return value of the member function call is placed in \a ret.
    You can pass up to ten arguments (val0, val1, val2, val3, val4, val5, val6, val7, val8, and val9)
    to the member function.

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
  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn, method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9, QString::fromLatin1(ret.name()));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  DEBUG << "Remote call" << method;
  return d->sendSynchronousRequest(request, ret);
}


/*!
    This function overloads call() method.
    This overload can be used if the return value of the member is of no interest.

    \note To set arguments, you must enclose them using Q_ARG macro.
    \note This method doesn't establish the connection to the server, you must use connectToServer() first.
    \sa callNoReply()
 */
bool CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn, method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  DEBUG << "Remote call" << method;
  return d->sendSynchronousRequest(request);
}



/*!
    Invokes the remote \a method (of the server). Returns true if the invokation was successful, false otherwise.
    Unlike the process of call() method, the invokation is asynchronous
    (which means that the client will not waiting for the response).

    The signature of this method is completely concurs with QMetaObject::invokeMethod() Qt method signature
    (without return value).
    Thus, you can use it the same way as you did it locally, with invokeMethod().

    You can pass up to ten arguments (val0, val1, val2, val3, val4, val5, val6, val7, val8, and val9)
    to the member function.

    \note To set arguments, you must enclose them using Q_ARG macro.
    \note This method doesn't establish the connection to the server, you must use connectToServer() first.
    \sa call(), connectToServer()
 */
void CuteIPCInterface::callNoReply(const QString& method, QGenericArgument val0, QGenericArgument val1,
                                        QGenericArgument val2, QGenericArgument val3, QGenericArgument val4,
                                        QGenericArgument val5, QGenericArgument val6, QGenericArgument val7,
                                        QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  CuteIPCMessage message(CuteIPCMessage::MessageCallWithoutReturn, method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  DEBUG << "Remote call (asynchronous)" << method;
  d->_q_sendAsynchronousRequest(request);
}


/*!
    Returns the error that last occured.
 */
QString CuteIPCInterface::lastError() const
{
  Q_D(const CuteIPCInterface);
  return d->m_lastError;
}
