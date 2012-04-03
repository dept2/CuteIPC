// Local
#include "CuteIPCServiceConnection_p.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QLocalSocket>
#include <QDataStream>
#include <QTime>
#include <QMetaType>


CuteIPCServiceConnection::CuteIPCServiceConnection(QLocalSocket* socket, CuteIPCService* parent)
  : QObject(parent),
    m_socket(socket),
    m_nextBlockSize(0)
{
  // Delete connection after the socket have been disconnected
  connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
  connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(errorOccured(QLocalSocket::LocalSocketError)));

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

    bool invokeWithReturn = false;
    int retType = 0;
    void* retData;
    if (!call.retType.isEmpty())
    {
      invokeWithReturn = true;
      retType = QMetaType::type(call.retType.toLatin1());
      if (retType == 0)
      {
        qWarning() << "Unsupported type of expected return value: " << call.retType << "Ignore returned value...";
        invokeWithReturn = false; // TODO quit (don't forget to clear the already allocated memory
      }
      else
      {
        // Read argument data from stream
        retData = QMetaType::construct(retType);
      }
    }

    qDebug() << "Before calling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

    if (invokeWithReturn)
    {
      bool successfulInvoke = QMetaObject::invokeMethod(parent(), call.first.toLatin1(), QGenericReturnArgument(call.retType.toLatin1(), retData), call.second.at(0),
                                          call.second.at(1), call.second.at(2), call.second.at(3),
                                          call.second.at(4), call.second.at(5), call.second.at(6),
                                          call.second.at(7), call.second.at(8), call.second.at(9));
      if (successfulInvoke)
        sendReturnedValue(QGenericArgument(call.retType.toLatin1(), retData)); //TODO check the scopes
    }
    else
    {
      QMetaObject::invokeMethod(parent(), call.first.toLatin1(), call.second.at(0),
                                call.second.at(1), call.second.at(2), call.second.at(3),
                                call.second.at(4), call.second.at(5), call.second.at(6),
                                call.second.at(7), call.second.at(8), call.second.at(9));
    }

    // Cleanup
    CuteIPCMarshaller::freeArguments(call.second);
    if (retType)
      QMetaType::destroy(retType, retData);

    m_nextBlockSize = 0;
    m_block.clear();
  }
}

void CuteIPCServiceConnection::sendReturnedValue(QGenericArgument arg)
{
  QByteArray request = CuteIPCMarshaller::marshallReturnedValue(arg);

  QDataStream stream(m_socket);
  stream << (quint32)request.size();
  int written = stream.writeRawData(request.constData(), request.size());

  if (written != request.size())
    qDebug() << "Written bytes and request size doesn't match";

  m_socket->flush();
  m_socket->waitForBytesWritten(5000);
  qDebug() << "Returned value was send";
}

void CuteIPCServiceConnection::errorOccured(QLocalSocket::LocalSocketError) {
  qDebug() << "Socket error: " << m_socket->errorString();
}
