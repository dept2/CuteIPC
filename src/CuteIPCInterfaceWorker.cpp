#include "CuteIPCInterfaceWorker.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCMarshaller_p.h"

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

    connect(m_connection, SIGNAL(socketDisconnected()), m_connection, SLOT(deleteLater()));
    connect(m_connection, SIGNAL(socketDisconnected()), m_socket, SLOT(deleteLater()));

    DEBUG << "CuteIPC:" << "Connected:" << name << connected;

    // Register connection ID on the serverside
    QString id = connectionId();
    CuteIPCMessage message(CuteIPCMessage::ConnectionInitialize, "", Q_ARG(QString, id));
    QByteArray request = CuteIPCMarshaller::marshallMessage(message);

    DEBUG << "Send connection ID to the server:" << id;

    QEventLoop loop;
    QObject::connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
    QObject::connect(m_connection, SIGNAL(socketDisconnected()), &loop, SLOT(quit()));
    m_connection->sendCallRequest(request);
    loop.exec();

    bool ok = m_connection->lastCallSuccessful();
    if (!ok)
      qWarning() << "CuteIPC:" << "Error: send connection ID failed. Remote signal connections will be unsuccessful";
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


QString CuteIPCInterfaceWorker::connectionId() const
{
  return QString::number(reinterpret_cast<quintptr>(m_connection.data()));
}
