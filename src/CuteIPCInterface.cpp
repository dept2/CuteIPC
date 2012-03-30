// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QLocalSocket>
#include <QTime>


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

  qDebug() << "Connected:" << connected;
  return connected;
}


void CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");
  QByteArray request = CuteIPCMarshaller::marshallCall(method, val0, val1, val2, val3, val4, val5, val6, val7, val8,
                                                       val9);

  qDebug() << "Trying to call" << method << "(method serialized into" << request.size() << "bytes)";

  QDataStream stream(m_socket);
  stream << (quint32)request.size();
  int written = stream.writeRawData(request.constData(), request.size());

  if (written != request.size())
    qDebug() << "Written bytes and request size doesn't match";

  m_socket->flush();
  bool ok = m_socket->waitForBytesWritten(5000);
  qDebug() << "Method called" << ok;
}
