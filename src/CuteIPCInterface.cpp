// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"

// Qt
#include <QLocalSocket>
#include <QMetaObject>
#include <QTime>
#include <QEventLoop>
#include <QMetaType>


CuteIPCInterface::CuteIPCInterface(QObject* parent)
  : QObject(parent),
    m_socket(new QLocalSocket(this))
{}


bool CuteIPCInterface::connectToServer(const QString& name)
{
  m_socket->connectToServer(name);
  bool connected = m_socket->waitForConnected(5000);
  if (!connected)
    m_socket->disconnectFromServer();

  if (connected)
  {
    m_connection = new CuteIPCInterfaceConnection(m_socket, this);
    qDebug() << "Connected:" << connected;
  }

  return connected;
}


bool CuteIPCInterface::connectRemoteSignal(const char* signal)
{
  qDebug() << "";
  qDebug() << "Requesting connection to signal" << signal;

  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, QString::fromAscii(signal));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  m_connection->sendCallRequest(request);
  return false;
}


bool CuteIPCInterface::call(const QString& method, QGenericReturnArgument ret, QGenericArgument val0,
                            QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4,
                            QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8,
                            QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9, QString::fromLatin1(ret.name()));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  m_connection->setReturnedObject(ret);
  qDebug() << "Trying to call" << method;

  return sendSynchronousRequest(request);
}


bool CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "Trying to call" << method;

  return sendSynchronousRequest(request);
}


bool CuteIPCInterface::sendSynchronousRequest(const QByteArray& request)
{
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);

  QEventLoop loop;
  connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
  loop.exec();

  return m_connection->lastCallSuccessful();
}


void CuteIPCInterface::callAsynchronous(const QString& method, QGenericArgument val0, QGenericArgument val1,
                                        QGenericArgument val2, QGenericArgument val3, QGenericArgument val4,
                                        QGenericArgument val5, QGenericArgument val6, QGenericArgument val7,
                                        QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithoutReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "Call (asynchronously)" << method;
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);
}


QString CuteIPCInterface::lastError() const
{
  return m_connection->lastError();
}
