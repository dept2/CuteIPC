// Local
#include "CuteIPCService.h"
#include "CuteIPCService_p.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCServiceConnection_p.h"
#include "CuteIPCSignalHandler_p.h"

// Qt
#include <QLocalServer>
#include <QLocalSocket>
#include <QTime>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

#include <QTcpServer>
#include <QTcpSocket>

#define CLIENTS_DISCONNECT_TIMEOUT 100

/*!
    \class CuteIPCService

    \brief The CuteIPCService class provides an IPC-server,
    based on Qt QLocalServer class and
    intended for recieving remote call requests and Qt signals.

    It can recieve messages from clients,
    such as method invoke request or Qt signal,
    process them and send a response back to the client.

    By inheriting the CuteIPCService class, you can invoke any method of your subclass,
    or to connect your local signal to the client's remote slot.

    Call listen() to have the server start listening.

    You can also connect client's signals and slots to the some other object
    (in this case, the CuteIPCService behaves only as adaptor).
    To make it possible, pass your object as argument in the listen() method.

    \sa CuteIPCClient
*/


CuteIPCServicePrivate::CuteIPCServicePrivate()
  : m_subject(0)
{}


CuteIPCServicePrivate::~CuteIPCServicePrivate()
{
  // Inform long-lived connections about connection closing
  QList<QObject*> connections = m_longLivedConnections.values();
  foreach (QObject* connection, connections)
  {
    qobject_cast<CuteIPCServiceConnection*>(connection)->sendAboutToQuit();
  }

  // Clients reaction timeout
  QEventLoop loop;
  QTimer timer;
  QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  timer.start(CLIENTS_DISCONNECT_TIMEOUT);
  loop.exec();
}


void CuteIPCServicePrivate::registerLocalServer()
{
  Q_Q(CuteIPCService);
  if (!m_localServer)
  {
    m_localServer = new QLocalServer(q);
    QObject::connect(m_localServer, SIGNAL(newConnection()), q, SLOT(_q_newLocalConnection()), Qt::UniqueConnection);
  }
}


void CuteIPCServicePrivate::registerTcpServer()
{
  Q_Q(CuteIPCService);
  if (!m_tcpServer)
  {
    m_tcpServer = new QTcpServer(q);
    QObject::connect(m_tcpServer, SIGNAL(newConnection()), q, SLOT(_q_newTcpConnection()), Qt::UniqueConnection);
  }
}


void CuteIPCServicePrivate::_q_newLocalConnection()
{
  Q_Q(CuteIPCService);
  QLocalSocket* socket = m_localServer->nextPendingConnection();
  Q_ASSERT(socket != 0);

  CuteIPCServiceConnection* connection = new CuteIPCServiceConnection(socket, q);
  connection->setSubject(m_subject);
}


void CuteIPCServicePrivate::_q_newTcpConnection()
{
  Q_Q(CuteIPCService);
  QTcpSocket* socket = m_tcpServer->nextPendingConnection();
  Q_ASSERT(socket != 0);

  CuteIPCServiceConnection* connection = new CuteIPCServiceConnection(socket, q);
  connection->setSubject(m_subject);
}


void CuteIPCServicePrivate::_q_handleSignalRequest(const QString& signalSignature, const QString& connectionId, QObject* sender)
{
  Q_Q(CuteIPCService);
  CuteIPCServiceConnection* senderConnection = qobject_cast<CuteIPCServiceConnection*>(sender);

  QObject* subject = m_subject ? m_subject : q;
  int signalIndex = subject->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toLatin1()));

  if (signalIndex == -1)
  {
    senderConnection->sendErrorMessage("Signal doesn't exist:" + signalSignature);
    return;
  }

  if (!m_longLivedConnections.contains(connectionId))
  {
    senderConnection->sendErrorMessage("Connection ID is not registered: " + connectionId);
    return;
  }

  CuteIPCSignalHandler* handler = m_signalHandlers.value(signalSignature);
  if (!handler)
  {
    //create a new signal handler
    handler = new CuteIPCSignalHandler(signalSignature, q);
    handler->setSignalParametersInfo(subject,signalSignature);
    m_signalHandlers.insert(signalSignature, handler);
    QMetaObject::connect(subject, subject->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toLatin1())),
                         handler, handler->metaObject()->indexOfSlot("relaySlot()"));
  }

//  handler->addListener(senderConnection);
  handler->addListener(qobject_cast<CuteIPCServiceConnection*>(m_longLivedConnections.value(connectionId)));
  senderConnection->sendResponseMessage(signalSignature);
}


void CuteIPCServicePrivate::_q_handleSignalDisconnect(const QString& signalSignature, const QString& connectionId, QObject* sender)
{
  CuteIPCServiceConnection* senderConnection = qobject_cast<CuteIPCServiceConnection*>(sender);

  CuteIPCSignalHandler* handler = m_signalHandlers.value(signalSignature);
  if (handler)
    handler->removeListener(qobject_cast<CuteIPCServiceConnection*>(m_longLivedConnections.value(connectionId)));

  senderConnection->sendResponseMessage(signalSignature);
}


void CuteIPCServicePrivate::_q_removeSignalHandler(QString key)
{
  m_signalHandlers.remove(key);
}


void CuteIPCServicePrivate::_q_initializeConnection(QString connectionId, QObject* sender)
{
  CuteIPCServiceConnection* senderConnection = qobject_cast<CuteIPCServiceConnection*>(sender);
  m_longLivedConnections.insert(connectionId, sender);
  senderConnection->sendResponseMessage("connectionInitialize_" + connectionId);
}


void CuteIPCServicePrivate::_q_connectionDestroyed(QObject* destroyedObject)
{
  QMutableHashIterator<QString, QObject*> i(m_longLivedConnections);
  while (i.hasNext())
  {
    i.next();
    if (i.value() == destroyedObject)
      i.remove();
  }
}


/*!
    Creates a new CuteIPCServer with the given \a parent.

    \sa listen()
 */
CuteIPCService::CuteIPCService(QObject* parent)
    : QObject(parent),
      d_ptr(new CuteIPCServicePrivate())
{
  Q_D(CuteIPCService);
  d->q_ptr = this;
}


CuteIPCService::CuteIPCService(CuteIPCServicePrivate& dd, QObject* parent)
    : QObject(parent),
      d_ptr(&dd)
{
  Q_D(CuteIPCService);
  d->q_ptr = this;
}


/*!
    Destroys the object.
 */
CuteIPCService::~CuteIPCService()
{
  delete d_ptr;
}


bool CuteIPCService::listenTcp(const QHostAddress& address, quint16 port, QObject* subject)
{
  Q_D(CuteIPCService);

  DEBUG << "Trying to listen" << address << "on port" << port;
  d->registerTcpServer();
  bool ok = d->m_tcpServer->listen(address, port);

  DEBUG << "CuteIPC:" << "Opened" << address << port << ok;
  d->m_subject = subject;
  return ok;
}


bool CuteIPCService::listenTcp(QObject* subject)
{
  return listenTcp(QHostAddress::Any, 0, subject);
}


quint16 CuteIPCService::tcpPort() const
{
  Q_D(const CuteIPCService);
  if (!d->m_tcpServer)
  {
    qWarning() << "CuteIPC: trying to get TCP port without listening being established";
    return -1;
  }

  return d->m_tcpServer->serverPort();
}


QHostAddress CuteIPCService::tcpAddress() const
{
  Q_D(const CuteIPCService);
  if (!d->m_tcpServer)
  {
    qWarning() << "CuteIPC: trying to get TCP address without listening being established";
    return QHostAddress::Null;
  }

  return d->m_tcpServer->serverAddress();
}


/*!
    Tells the server to listen for incoming connections on \a serverName.
    If the serverName is an empty string, it generates the name based on class name.
    If the server is currently listening then it will try
    to reuse the existing pipe.
    \param subject Sets the object, which methods will be invoked (by default, it well be the CuteIPCService
    class itself)
    Return true on success, otherwise false.
 */
bool CuteIPCService::listen(const QString& serverName, QObject* subject)
{
  Q_D(CuteIPCService);
  QString name = serverName;
  if (name.isEmpty())
    name = QString(QLatin1String("%1.%2")).arg(metaObject()->className()).arg(reinterpret_cast<quintptr>(this));

  DEBUG << "Trying to listen" << name;
  d->registerLocalServer();
  bool ok = d->m_localServer->listen(name);

  if (!ok)
  {
    DEBUG << "Trying to reuse existing pipe";
    ok = d->m_localServer->removeServer(name);
    if (ok)
    {
      DEBUG << "Server removed, connecting again";
      ok = d->m_localServer->listen(name);
    }
  }

  DEBUG << "CuteIPC:" << "Opened" << name << ok;
  d->m_subject = subject;

  return ok;
}


bool CuteIPCService::listen(QObject* subject)
{
  return listen(QString(), subject);
}


/*!
    Stop listening for incoming connections.
    Existing connections are not effected, but any new connections will be refused.
 */
void CuteIPCService::close()
{
  Q_D(CuteIPCService);
  if (d->m_localServer)
    d->m_localServer->close();
  else if (d->m_tcpServer)
    d->m_tcpServer->close();
}


/*!
    Returns the server name if the server is listening for connections;
    otherwise returns QString()

    \sa listen()
 */
QString CuteIPCService::serverName() const
{
  Q_D(const CuteIPCService);
  if (!d->m_localServer)
  {
    qWarning() << "CuteIPC: trying to get local server name without listening being established";
    return QString();
  }

  return d->m_localServer->serverName();
}

/*! \mainpage CuteIPC
 *
 * \section intro_sec Introduction
 *
 * The CuteIPC is a Qt library which is intended to
 * make remote method calls and to transmit Qt signals
 * through the local socket connection.
 *
 * It's based on QLocalSocket and QLocalServer Qt classes, thus adopts Qt
 * platform independence.
 *
 *
 * \section usage_sec Usage
 *
 * To use the library, just inherit the CuteIPCService class on the server side
 * and the CuteIPCInterface class on the client.
 * It makes it possible to call the methods of class that inherits the CuteIPCService
 * from the client.
 * You can also connect client's slots and signals to some other object on the server side
 * (in this case, the CuteIPCService class behaves as adaptor)
 *
 * The signature of the CuteIPCInterface call methods is completely concurs
 * with \a QMetaObject::invokeMethod() Qt method signature.
 *
 * You can also remotely connect the server-side signals to the client's slots and signals.
 * On the other hand, you can connect client's local signals to the remote slots of
 * the server.
 *
 * \note To enable debug output, set CUTEIPC_DEBUG environment variable to 1;
 *
 */
#include "moc_CuteIPCService.cpp"
