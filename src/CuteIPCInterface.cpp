// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QLocalSocket>
#include <QMetaType>
#include <QTime>


CuteIPCInterface::CuteIPCInterface(QObject* parent)
  : QObject(parent),
    m_socket(new QLocalSocket(this)),
    m_nextBlockSize(0)
{
  connect(m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
}


bool CuteIPCInterface::connectToServer(const QString& name)
{
  m_socket->connectToServer(name);
  bool connected = m_socket->waitForConnected(5000);
  if (!connected)
    m_socket->disconnectFromServer();

  qDebug() << "Connected:" << connected;
  return connected;
}


bool CuteIPCInterface::call(const QString& method, QString returnType, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  bool callWithReturn = returnType.isEmpty() ? false : true;
  qDebug() << callWithReturn;

  QByteArray request = CuteIPCMarshaller::marshallCall(method, val0, val1, val2, val3, val4, val5, val6, val7, val8, val9, returnType);

  qDebug() << "Trying to call" << method << "(method serialized into" << request.size() << "bytes)";

  QDataStream stream(m_socket);
  stream << (quint32)request.size();
  int written = stream.writeRawData(request.constData(), request.size());

  if (written != request.size())
    qDebug() << "Written bytes and request size doesn't match";

  m_socket->flush();
  bool ok = m_socket->waitForBytesWritten(5000);
  qDebug() << "Method called" << ok;

  if (callWithReturn)
  {
    qDebug() << "Wait for returned value...";
    ok = m_socket->waitForReadyRead(30000);
    qDebug() << "Returned value was read" << ok;
    return ok;
  }

  return true;
}

void CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  call(method, QString(), val0, val1, val2, val3, val4, val5, val6, val7, val8, val9);
}

void CuteIPCInterface::readyRead()
{
  QDataStream in(m_socket);

  // Fetch next block size
  if (m_nextBlockSize == 0)
  {
    qDebug() << "Started fetching request for returned value:" << QTime::currentTime().toString("hh:mm:ss.zzz");
    if (m_socket->bytesAvailable() < (int)sizeof(quint32))
      return;

    in >> m_nextBlockSize;
  }

  if (in.atEnd())
    return;

  qint64 bytesToFetch = m_nextBlockSize - m_block.size();
  m_block.append(m_socket->read(bytesToFetch));

  if (m_block.size() == (int)m_nextBlockSize)
  {
    // Fetched enough, need to parse
    qDebug() << "Returned value: Fetching block finished. Got" << m_block.size() << "bytes:" << QTime::currentTime().toString("hh:mm:ss.zzz");
    QGenericArgument ret = CuteIPCMarshaller::demarshallReturnedValue(m_block);
    qDebug() << "SUCCESS: returned value type: " << QString::fromLatin1(ret.name()) << " value: "
                << *((int*)ret.data());

    m_nextBlockSize = 0;
    m_block.clear();
    emit returnValue(ret);
  }
}
