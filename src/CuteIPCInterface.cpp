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


CuteIPCInterfacePrivate::CuteIPCInterfacePrivate()
    : m_socket(0)
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
    qDebug() << "incompatible signatures" << signalSignature << slotSignature;
    return false;
  }
  return true;
}


bool CuteIPCInterfacePrivate::sendRemoteConnectionRequest(const QString &signal)
{
  qDebug() << "Requesting connection to signal" << signal;
  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, signal);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);
  bool ok = sendSynchronousRequest(request);
  return ok;
}


bool CuteIPCInterfacePrivate::sendSynchronousRequest(const QByteArray& request)
{
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);

  QEventLoop loop;
  QObject::connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
  loop.exec();

  qDebug() << "call was finished. stop local event loop...";

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
  qDebug() << "!REMOVE! remove remote signal connections of object:" << destroyedObject;

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
  MethodData data = m_connections.value(signalSignature);
  if (!data.first)
    return;

  qDebug() << "SIGNAL: Trying to invoke slot";

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
  qDebug() << "SIGNAL: invoke slot:" << successfulInvoke;
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
    qDebug() << "Create a new local signal handler for the signature: " << signalSignature;
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
  qDebug() << "!REMOVE! remove signal handlers of object:" << destroyedObject;

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
  qDebug() << "Send signal...";
  m_connection->sendCallRequest(request);
}


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


CuteIPCInterface::~CuteIPCInterface()
{}


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


bool CuteIPCInterface::remoteConnect(const char* signal, QObject* object, const char* slot)
{
  Q_D(CuteIPCInterface);
  QString signalSignature = QString::fromAscii(signal);
  QString slotSignature = QString::fromAscii(slot);

  if (!d->checkConnectCorrection(signalSignature, slotSignature))
    return false;

  signalSignature = signalSignature.mid(1);
  slotSignature = slotSignature.mid(1);

  if (!d->sendRemoteConnectionRequest(signalSignature))
    return false;

  d->registerConnection(signalSignature, object, slotSignature);
  return true;
}


bool CuteIPCInterface::remoteConnect(QObject *localObject, const char *signal, const char *remoteSlot)
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
    qDebug() << "Signal doesn't exist:" + signalSignature << "object:" << localObject;
    return false;
  }

  d->handleLocalSignalRequest(localObject, signalSignature, slotSignature);
  return true;
}


bool CuteIPCInterface::call(const QString& method, QGenericReturnArgument ret, QGenericArgument val0,
                            QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4,
                            QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8,
                            QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9, QString::fromLatin1(ret.name()));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  d->m_connection->setReturnedObject(ret);
  qDebug() << "Trying to call" << method;

  return d->sendSynchronousRequest(request);
}


bool CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "Trying to call" << method;

  return d->sendSynchronousRequest(request);
}



void CuteIPCInterface::callNoReply(const QString& method, QGenericArgument val0, QGenericArgument val1,
                                        QGenericArgument val2, QGenericArgument val3, QGenericArgument val4,
                                        QGenericArgument val5, QGenericArgument val6, QGenericArgument val7,
                                        QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithoutReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "Call (asynchronously)" << method;
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  d->m_connection->sendCallRequest(request);
}


QString CuteIPCInterface::lastError() const
{
  Q_D(const CuteIPCInterface);
  return d->m_connection->lastError();
}
