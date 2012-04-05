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
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(errorOccured(QLocalSocket::LocalSocketError)));

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
    qDebug() << "";
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
    qDebug() << "Fetching block finished. Got" << m_block.size() << "bytes:"
             << QTime::currentTime().toString("hh:mm:ss.zzz");

    makeCall();

    // Cleanup
    m_nextBlockSize = 0;
    m_block.clear();
  }
}


void CuteIPCServiceConnection::makeCall()
{
  CuteIPCMarshaller::Call call = CuteIPCMarshaller::demarshallCall(m_block);

  if (call.calltype == CuteIPCMarshaller::CALL_WITH_RETURN) //!call.retType.isEmpty())
  {
    int retType = QMetaType::type(call.retType.toLatin1());
    if (retType > 0)
    {
      // Read argument data from stream
      void* retData = QMetaType::construct(retType);

      qDebug() << "Before calling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

      bool successfulInvoke = QMetaObject::invokeMethod(parent(), call.first.toLatin1(),
                                                        QGenericReturnArgument(call.retType.toLatin1(), retData),
                                                        call.second.at(0), call.second.at(1), call.second.at(2),
                                                        call.second.at(3), call.second.at(4), call.second.at(5),
                                                        call.second.at(6), call.second.at(7), call.second.at(8),
                                                        call.second.at(9));

      if (successfulInvoke)
      {
        qDebug() << "Method was successfully invoked";
        sendReturnedValue(QGenericArgument(call.retType.toLatin1(), retData)); //TODO check the scopes
      }
      else
      {
        sendErrorMessage("Unsuccessful invoke");
      }

      QMetaType::destroy(retType, retData);
    }
    else
    {
      QString error = "Unsupported type of expected return value: " + call.retType;
      qWarning() << error;
      sendErrorMessage(error);
    }
  }
  else // CALL_WITH_CONFIRM or CALL_WITHOUT_CONFIRM
  {
    bool successfulInvoke = QMetaObject::invokeMethod(parent(), call.first.toLatin1(),
                                                      call.second.at(0), call.second.at(1), call.second.at(2),
                                                      call.second.at(3), call.second.at(4), call.second.at(5),
                                                      call.second.at(6), call.second.at(7), call.second.at(8),
                                                      call.second.at(9));
    if (!successfulInvoke)
    {
      sendErrorMessage("Unsuccessful invoke");
    }
    else
    {
      qDebug() << "Method was successfully invoked";
      if (call.calltype == CuteIPCMarshaller::CALL_WITH_CONFIRM)
        sendConfirm();
    }
  }

  // Cleanup
  CuteIPCMarshaller::freeArguments(call.second);
}


void CuteIPCServiceConnection::sendConfirm()
{
  CuteIPCMarshaller::Status status;
  status.first = true;

  QByteArray request = CuteIPCMarshaller::marshallStatusMessage(status);
  sendResponse(request);
  qDebug() << "Status was sent";
}


void CuteIPCServiceConnection::sendErrorMessage(QString error)
{
  CuteIPCMarshaller::Status status;
  status.first = false;
  status.second = error;

  QByteArray request = CuteIPCMarshaller::marshallStatusMessage(status);
  sendResponse(request);
  qDebug() << "Error message was sent";
}


void CuteIPCServiceConnection::sendReturnedValue(QGenericArgument arg)
{
  QByteArray request = CuteIPCMarshaller::marshallReturnedValue(arg);
  sendResponse(request);
  qDebug() << "Returned value was sent";
}


void CuteIPCServiceConnection::sendResponse(QByteArray request)
{
  QDataStream stream(m_socket);
  stream << (quint32)request.size();
  int written = stream.writeRawData(request.constData(), request.size());

  if (written != request.size())
    qDebug() << "Written bytes and request size doesn't match";

  m_socket->flush();
}


void CuteIPCServiceConnection::errorOccured(QLocalSocket::LocalSocketError)
{
  qDebug() << "Socket error: " << m_socket->errorString();
}
