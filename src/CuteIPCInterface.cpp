// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCInterfaceConnection_p.h"

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


bool CuteIPCInterface::call(const QString& method, QGenericReturnArgument ret, QGenericArgument val0,
                            QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4,
                            QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8,
                            QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");
  QByteArray request = CuteIPCMarshaller::marshallCall(method, val0, val1, val2, val3, val4,
                                                       val5, val6, val7, val8, val9,
                                                       QString::fromLatin1(ret.name()));

  m_connection->setReturnedObject(ret);
  qDebug() << "Trying to call" << method;

  sendSynchronousRequest(request);
  return true;
}


void CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");
  QByteArray request = CuteIPCMarshaller::marshallCall(method, val0, val1, val2, val3, val4,
                                                       val5, val6, val7, val8, val9, QString());
  qDebug() << "Trying to call" << method;

  sendSynchronousRequest(request);
}


void CuteIPCInterface::sendSynchronousRequest(const QByteArray& request)
{
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);

  QEventLoop loop;
  connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
  loop.exec();
}


void CuteIPCInterface::callAsynchronous(const QString &method, QGenericArgument val0, QGenericArgument val1,
                                               QGenericArgument val2, QGenericArgument val3, QGenericArgument val4,
                                               QGenericArgument val5, QGenericArgument val6, QGenericArgument val7,
                                               QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");
  QByteArray request = CuteIPCMarshaller::marshallCall(method, val0, val1, val2, val3, val4,
                                                       val5, val6, val7, val8, val9, QString(), false);
  qDebug() << "Call (asynchronously)" << method;
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);
}
