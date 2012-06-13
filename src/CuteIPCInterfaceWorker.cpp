#include "CuteIPCInterfaceWorker.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"

// Qt
#include <QEventLoop>


CuteIPCInterfaceWorker::CuteIPCInterfaceWorker(QObject* parent)
  : QObject(parent)
{}


CuteIPCInterfaceWorker::~CuteIPCInterfaceWorker()
{
  if (m_socket)
    m_socket->deleteLater();
}


void CuteIPCInterfaceWorker::registerSocket()
{
  m_socket = new QLocalSocket;
  emit registerSocketFinished();
}


void CuteIPCInterfaceWorker::connectToServer(const QString& name, void* successful)
{
  m_socket->connectToServer(name);
  bool connected = m_socket->waitForConnected(5000);
  if (!connected)
    m_socket->disconnectFromServer();

  if (connected)
  {
    m_connection = new CuteIPCInterfaceConnection(m_socket, this);
    connect(m_connection, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)),
            this, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)));
    connect(m_connection, SIGNAL(errorOccured(QString)), this, SIGNAL(setLastError(QString)));

    DEBUG << "CuteIPC:" << "Connected:" << name << connected;
  }

  *reinterpret_cast<bool*>(successful) = connected;
  emit connectToServerFinished();
}


void CuteIPCInterfaceWorker::disconnectFromServer()
{
  m_socket->disconnectFromServer();
  delete m_socket;
  registerSocket();

  emit disconnectFromServerFinished();
}


void CuteIPCInterfaceWorker::sendCallRequest(const QByteArray& request)
{
  if (!m_connection)
    return;

  m_connection->sendCallRequest(request);
}
