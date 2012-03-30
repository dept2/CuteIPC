// Local
#include "CuteIPCServiceConnection_p.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QLocalSocket>
#include <QDataStream>
#include <QTime>


CuteIPCServiceConnection::CuteIPCServiceConnection(QLocalSocket* socket, CuteIPCService* parent)
  : QObject(parent),
    m_socket(socket),
    m_nextBlockSize(0)
{
  // Delete connection after the socket have been disconnected
  connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
  connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));

  connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
  if (!socket->open(QIODevice::ReadWrite))
  {
    qWarning() << "Failed to open socket in ReadWrite mode:" << socket->errorString();
    deleteLater();
  }
}


void CuteIPCServiceConnection::readyRead()
{
  QDataStream in(m_socket);

  // Fetch next block size
  if (m_nextBlockSize == 0)
  {
    qDebug() << "Started fetching request:" << QTime::currentTime().toString("hh:mm:ss.zzz");
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
    qDebug() << "Fetching block finished. Got" << m_block.size() << "bytes:" << QTime::currentTime().toString("hh:mm:ss.zzz");

    CuteIPCMarshaller::Call call = CuteIPCMarshaller::demarshallCall(m_block);

    qDebug() << "Before calling:" << QTime::currentTime().toString("hh:mm:ss.zzz");
    QMetaObject::invokeMethod(parent(), call.first.toLatin1(), call.second.at(0), call.second.at(1), call.second.at(2),
    call.second.at(3), call.second.at(4), call.second.at(5), call.second.at(6),
    call.second.at(7), call.second.at(8), call.second.at(9));
    qDebug() << "Method" << call.first << "called";

    CuteIPCMarshaller::freeArguments(call.second);

    // Cleanup
    m_nextBlockSize = 0;
    m_block.clear();
  }
}
