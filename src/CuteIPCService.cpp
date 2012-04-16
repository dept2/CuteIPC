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


CuteIPCServicePrivate::CuteIPCServicePrivate()
    : m_server(0)
{}


CuteIPCServicePrivate::~CuteIPCServicePrivate()
{}


void CuteIPCServicePrivate::registerServer()
{
  Q_Q(CuteIPCService);
  m_server = new QLocalServer(q);
  QObject::connect(m_server, SIGNAL(newConnection()), q, SLOT(_q_newConnection()));
}


void CuteIPCServicePrivate::_q_newConnection()
{
  Q_Q(CuteIPCService);
  QLocalSocket* socket = m_server->nextPendingConnection();
  Q_ASSERT(socket != 0);

  new CuteIPCServiceConnection(socket, q);
}


void CuteIPCServicePrivate::_q_handleSignalRequest(QString signalSignature, QObject* sender)
{
  Q_Q(CuteIPCService);
//  qDebug() << Q_FUNC_INFO;

  CuteIPCServiceConnection* senderConnection = qobject_cast<CuteIPCServiceConnection*>(sender);

  int signalIndex = q->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toAscii()));
//  qDebug() << "SIGNAL INDEX: " << signalIndex;
  if (signalIndex == -1)
  {
    senderConnection->sendErrorMessage("Signal doesn't exist:" + signalSignature);
    return;
  }


  CuteIPCSignalHandler* handler = m_signalHandlers.value(signalSignature);
  if (!handler)
  {
    //create a new signal handler
    qDebug() << "ACTION: Create a new signal handler for the signature: " << signalSignature;
    handler = new CuteIPCSignalHandler(signalSignature, q);
    m_signalHandlers.insert(signalSignature, handler);


    QMetaObject::connect(q,
                         q->metaObject()->indexOfSignal(
                             QMetaObject::normalizedSignature(signalSignature.toAscii())),
                         handler,
                             handler->metaObject()->indexOfSlot("relaySlot()"));
  }

  handler->addListener(senderConnection);
  senderConnection->sendResponseMessage(signalSignature);
}


void CuteIPCServicePrivate::_q_removeSignalHandler(QString key)
{
  m_signalHandlers.remove(key);
}


CuteIPCService::CuteIPCService(QObject *parent)
    : QObject(parent),
      d_ptr(new CuteIPCServicePrivate())
{
  Q_D(CuteIPCService);
  d->q_ptr = this;
  d->registerServer();
}


CuteIPCService::CuteIPCService(CuteIPCServicePrivate& dd, QObject* parent)
    : QObject(parent),
      d_ptr(&dd)
{
  Q_D(CuteIPCService);
  d->q_ptr = this;
  d->registerServer();
}


CuteIPCService::~CuteIPCService()
{}


bool CuteIPCService::listen(const QString& serverName)
{
  Q_D(CuteIPCService);
  QString name = serverName;
  if (name.isEmpty())
    name = QString(QLatin1String("%1.%2")).arg(metaObject()->className()).arg(reinterpret_cast<quintptr>(this));

  qDebug() << "Trying to listen" << name;
  bool ok = d->m_server->listen(name);

  if (!ok)
  {
    qDebug() << "Trying to reuse existing pipe";
    ok = d->m_server->removeServer(name);
    if (ok)
    {
      qDebug("Server removed, connecting again");
      ok = d->m_server->listen(name);
    }
  }

  qDebug() << "Opened" << ok;

  return ok;
}


QString CuteIPCService::serverName() const
{
  Q_D(const CuteIPCService);
  return d->m_server->serverName();
}
