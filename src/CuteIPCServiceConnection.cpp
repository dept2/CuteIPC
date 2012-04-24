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
  connect(this, SIGNAL(signalRequest(QString, QObject*)), parent, SLOT(_q_handleSignalRequest(QString, QObject*)));
  connect(this, SIGNAL(signalDisconnectRequest(QString,QObject*)), parent, SLOT(_q_handleSignalDisconnect(QString,QObject*)));

  connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
  if (!socket->open(QIODevice::ReadWrite))
  {
    qWarning() << "Failed to open socket in ReadWrite mode:" << socket->errorString();
    deleteLater();
  }
}


CuteIPCServiceConnection::~CuteIPCServiceConnection()
{}


void CuteIPCServiceConnection::readyRead()
{
  bool messageStreamFinished;

  do
  {
    messageStreamFinished = readMessageFromSocket();
  } while (!messageStreamFinished);
}

bool CuteIPCServiceConnection::readMessageFromSocket()
{
  QDataStream in(m_socket);

  // Fetch next block size
  if (m_nextBlockSize == 0)
  {
//    qDebug() << "";
//    qDebug() << "Started fetching request:" << QTime::currentTime().toString("hh:mm:ss.zzz");
    if (m_socket->bytesAvailable() < (int)sizeof(quint32))
      return true;

    in >> m_nextBlockSize;
  }

  if (in.atEnd())
    return true;

  qint64 bytesToFetch = m_nextBlockSize - m_block.size();
  m_block.append(m_socket->read(bytesToFetch));

  if (m_block.size() == (int)m_nextBlockSize)
  {
    // Fetched enough, need to parse
//    qDebug() << "Fetching block finished. Got" << m_block.size() << "bytes:"
//             << QTime::currentTime().toString("hh:mm:ss.zzz");

    processMessage();

    // Cleanup
    m_nextBlockSize = 0;
    m_block.clear();
  }

  if (m_socket->bytesAvailable())
    return false;
  return true;
}


void CuteIPCServiceConnection::processMessage()
{
  CuteIPCMessage call = CuteIPCMarshaller::demarshallMessage(m_block);
  CuteIPCMessage::MessageType messageType = call.messageType();
  qDebug() << "";
  qDebug() << call;

  // Fill empty args
  CuteIPCMessage::Arguments args = call.arguments();
  while (args.size() < 10)
    args.append(QGenericArgument());

  if (messageType == CuteIPCMessage::MessageCallWithReturn && !call.returnType().isEmpty())
  {
    int retType = QMetaType::type(call.returnType().toLatin1());
    if (retType > 0)
    {
      // Read argument data from stream
      void* retData = QMetaType::construct(retType);

//      qDebug() << "Before calling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

      bool successfulInvoke = QMetaObject::invokeMethod(parent(), call.method().toLatin1(),
          QGenericReturnArgument(call.returnType().toLatin1(), retData),
          args.at(0), args.at(1), args.at(2),
          args.at(3), args.at(4), args.at(5),
          args.at(6), args.at(7), args.at(8),
          args.at(9));

      if (successfulInvoke)
      {
        qDebug() << "ACTION: Method was successfully invoked";
        sendResponseMessage(call.method(), QGenericArgument(call.returnType().toLatin1(), retData));
      }
      else
      {
        sendErrorMessage("Unsuccessful invoke");
      }

      QMetaType::destroy(retType, retData);
    }
    else
    {
      QString error = "Unsupported type of expected return value: " + call.returnType();
      qWarning() << error;
      sendErrorMessage(error);
    }
  }
  else if ((messageType == CuteIPCMessage::MessageCallWithReturn && call.returnType().isEmpty())
           || messageType == CuteIPCMessage::MessageCallWithoutReturn)
  {
    bool successfulInvoke = QMetaObject::invokeMethod(parent(), call.method().toLatin1(),
        args.at(0), args.at(1), args.at(2),
        args.at(3), args.at(4), args.at(5),
        args.at(6), args.at(7), args.at(8),
        args.at(9));
    if (!successfulInvoke)
    {
      sendErrorMessage("Unsuccessful invoke");
    }
    else
    {
      qDebug() << "ACTION: Method was successfully invoked";
      if (messageType == CuteIPCMessage::MessageCallWithReturn)
        sendResponseMessage(call.method());
    }
  }
  else if (messageType == CuteIPCMessage::MessageSignal)
  {
    bool successfulInvoke = QMetaObject::invokeMethod(parent(),
        call.method().left(call.method().indexOf("(")).toLatin1(),
        args.at(0), args.at(1), args.at(2),
        args.at(3), args.at(4), args.at(5),
        args.at(6), args.at(7), args.at(8),
        args.at(9));
    if (!successfulInvoke)
      sendErrorMessage("Unsuccessful invoke");
    else
      qDebug() << "ACTION: Method was successfully invoked";
  }
  else if (messageType == CuteIPCMessage::SignalConnectionRequest)
  {
    if (call.returnType() != QString("disconnect"))
      emit signalRequest(call.method(), this);
    else
      emit signalDisconnectRequest(call.method(), this);
  }
  else if (messageType == CuteIPCMessage::SlotConnectionRequest)
  {
    if (parent()->metaObject()->indexOfSlot(QMetaObject::normalizedSignature(call.method().toAscii())) == -1)
      sendErrorMessage("Remote slot doesn't exist:" + call.method());
    else
      sendResponseMessage(call.method());
  }

  // Cleanup
  CuteIPCMarshaller::freeArguments(call.arguments());
}


void CuteIPCServiceConnection::sendErrorMessage(const QString& error)
{
  CuteIPCMessage message(CuteIPCMessage::MessageError, error);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);
  sendResponse(request);
  qDebug() << "REMOTE ACTION: Error message was sent";
}


void CuteIPCServiceConnection::sendResponseMessage(const QString& method, QGenericArgument arg)
{
  CuteIPCMessage message(CuteIPCMessage::MessageResponse, method, arg);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  sendResponse(request);
  qDebug() << "REMOTE ACTION: Returned value was sent";
}


void CuteIPCServiceConnection::sendResponse(const QByteArray& response)
{
  QDataStream stream(m_socket);
  stream << (quint32)response.size();
  int written = stream.writeRawData(response.constData(), response.size());

  if (written != response.size())
    qDebug() << "ERROR: Written bytes and request size doesn't match";

  m_socket->flush();
}


void CuteIPCServiceConnection::errorOccured(QLocalSocket::LocalSocketError)
{
  qDebug() << "Socket error: " << m_socket->errorString();
  deleteLater();
}


void CuteIPCServiceConnection::sendSignal(const QByteArray &data)
{
  sendResponse(data);
  qDebug() << "REMOTE ACTION: Signal was sent";
}
