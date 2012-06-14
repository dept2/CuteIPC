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
            this, SLOT(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)));
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


void CuteIPCInterfaceWorker::remoteConnect(const QString& signalSignature, void* object, const QString& methodSignature)
{
  if (!m_connections.contains(signalSignature))
      sendRemoteConnectionRequest(signalSignature);

  registerConnection(signalSignature, reinterpret_cast<QObject*>(object), methodSignature);
}


void CuteIPCInterfaceWorker::disconnectSignal(const QString& signalSignature, void* object, const QString& methodSignature)
{
  m_connections.remove(signalSignature, MethodData(reinterpret_cast<QObject*>(object), methodSignature));
  if (!m_connections.contains(signalSignature))
      sendSignalDisconnectRequest(signalSignature);
}


void CuteIPCInterfaceWorker::sendRemoteConnectionRequest(const QString& signal)
{
  DEBUG << "Requesting connection to signal" << signal;
  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, signal);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  sendCallRequest(request);
}


void CuteIPCInterfaceWorker::sendSignalDisconnectRequest(const QString& signal)
{
  DEBUG << "Requesting remote signal disconnect" << signal;
  CuteIPCMessage::Arguments args;
  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, signal, args, "disconnect");
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  sendCallRequest(request);
}


void CuteIPCInterfaceWorker::registerConnection(const QString& signalSignature, QObject* reciever, const QString& methodSignature)
{
  m_connections.insert(signalSignature, MethodData(reciever, methodSignature));
  QObject::connect(reciever, SIGNAL(destroyed(QObject*)), this, SLOT(removeRemoteConnectionsOfObject(QObject*)));
}


void CuteIPCInterfaceWorker::removeRemoteConnectionsOfObject(QObject* destroyedObject)
{
  QMutableHashIterator<QString, MethodData> i(m_connections);
  while (i.hasNext())
  {
    i.next();
    MethodData data = i.value();
    if (data.first == destroyedObject)
      i.remove();
  }
}


void CuteIPCInterfaceWorker::invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments)
{
  QList<MethodData> recieversData = m_connections.values(signalSignature);
  foreach (const MethodData& data, recieversData)
  {
    if (!data.first)
      return;

    DEBUG << "Invoke local method: " << data.second;

    QString methodName = data.second;
    methodName = methodName.left(methodName.indexOf("("));

    CuteIPCMessage::Arguments args = arguments;
    while (args.size() < 10)
      args.append(QGenericArgument());

    QMetaObject::invokeMethod(data.first, methodName.toAscii(), Qt::QueuedConnection,
                              args.at(0), args.at(1), args.at(2), args.at(3), args.at(4), args.at(5), args.at(6),
                              args.at(7), args.at(8), args.at(9));
  }
}
