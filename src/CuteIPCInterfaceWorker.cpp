#include "CuteIPCInterfaceWorker.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCLoopVector.h"

// Qt
#include <QObject>
#include <QEventLoop>


CuteIPCInterfaceWorker::CuteIPCInterfaceWorker(QObject* parent)
  : QObject(parent),
    m_socket(0),
    m_syncCallLoops(0)
{}


CuteIPCInterfaceWorker::~CuteIPCInterfaceWorker()
{
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

    m_syncCallLoops = new CuteIPCLoopVector(m_connection, SIGNAL(callFinished()), m_connection); //!!!!! Check m_connection as parent!!!!!

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


void CuteIPCInterfaceWorker::sendSynchronousRequest(const QByteArray& request, void* successful, QGenericReturnArgument returnedObject)
{
  if (!m_connection)
  {
    *reinterpret_cast<bool*>(successful) = false;
    emit sendSynchronousRequestFinished();
    return;
  }

  m_connection->setReturnedObject(returnedObject);

  m_syncCallLoops->append();
  m_connection->sendCallRequest(request);
  m_syncCallLoops->exec();

  *reinterpret_cast<bool*>(successful) = m_connection->lastCallSuccessful();
  emit sendSynchronousRequestFinished();
}


void CuteIPCInterfaceWorker::sendSignal(const QByteArray& request)
{
  if (!m_connection)
    return;

  m_connection->sendCallRequest(request);
}


void CuteIPCInterfaceWorker::sendCallRequest(const QByteArray& request)
{
  if (!m_connection)
    return;

  m_connection->sendCallRequest(request);
}
