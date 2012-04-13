// Local
#include "CuteIPCInterface.h"
#include "CuteIPCInterface_p.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"

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


bool CuteIPCInterfacePrivate::checkConnectCorrection(const QString& signal, const QObject* object, const QString& slot)
{
  //check signal and slot existing
  if (!object || object->metaObject()->indexOfSlot(slot.toAscii()) == -1)
  {
    qDebug() << "incorrect connect reciever";
    return false;
  }

  if (!QMetaObject::checkConnectArgs(signal.toAscii(), slot.toAscii()))
  {
    qDebug() << "incompatible signatures" << signal << slot;
    return false;
  }

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
  m_connections.insert(signalSignature, SlotData(reciever, slotSignature));
}


void CuteIPCInterfacePrivate::removeConnection(const QString& signalSignature)
{
  m_connections.remove(signalSignature);
}


void CuteIPCInterfacePrivate::_q_invokeRemoteSignal(const QString& signalSignature,
                                                    const CuteIPCMessage::Arguments& arguments)
{
  SlotData data = m_connections.value(signalSignature);
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
  if (signal[0] != '2' || slot[0] != '1')
    return false;

  QString signalSignature = QString::fromAscii(signal).mid(1);
  QString slotSignature = QString::fromAscii(slot).mid(1);

  if (!d->checkConnectCorrection(signalSignature, object, slotSignature))
    return false;

  d->registerConnection(signalSignature, object, slotSignature);
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
