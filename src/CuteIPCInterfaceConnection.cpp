// Local
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QLocalSocket>
#include <QDataStream>
#include <QTime>
#include <QMetaType>


CuteIPCInterfaceConnection::CuteIPCInterfaceConnection(QLocalSocket* socket, CuteIPCInterface* parent)
  : QObject(parent),
    m_socket(socket),
    m_nextBlockSize(0)
{
  // Delete connection after the socket have been disconnected
  connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
  connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(errorOccured(QLocalSocket::LocalSocketError)));

  connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
}


void CuteIPCInterfaceConnection::sendCallRequest(QByteArray request)
{
  QDataStream stream(m_socket);
  stream << (quint32)request.size();
  int written = stream.writeRawData(request.constData(), request.size());

  if (written != request.size())
    qDebug() << "Written bytes and request size doesn't match";

  m_socket->flush();
  bool ok = m_socket->waitForBytesWritten(5000);
  qDebug() << "Method called" << ok;
}


void CuteIPCInterfaceConnection::readyRead()
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
    CuteIPCMarshaller::demarshallReturnedValue(m_block, m_returnedObject);

    m_nextBlockSize = 0;
    m_block.clear();

    emit callFinished();
  }
}


void CuteIPCInterfaceConnection::errorOccured(QLocalSocket::LocalSocketError) {
  qDebug() << "Socket error: " << m_socket->errorString();
}


void CuteIPCInterfaceConnection::setReturnedObject(QGenericReturnArgument returnedObject)
{
  m_returnedObject = returnedObject;
}
