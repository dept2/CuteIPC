// Local
#include "CuteIPCService.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCServiceConnection_p.h"
#include "CuteIPCSignalHandler_p.h"

// Qt
#include <QLocalServer>
#include <QLocalSocket>
#include <QTime>
#include <QDebug>


CuteIPCService::CuteIPCService(QObject* parent)
  : QObject(parent),
    m_server(new QLocalServer(this))
{
  connect(m_server, SIGNAL(newConnection()), SLOT(newConnection()));
}


CuteIPCService::~CuteIPCService()
{}


bool CuteIPCService::listen(const QString& serverName)
{
  QString name = serverName;
  if (name.isEmpty())
    name = QString(QLatin1String("%1.%2")).arg(metaObject()->className()).arg(reinterpret_cast<quintptr>(this));

  qDebug() << "Trying to listen" << name;
  bool ok = m_server->listen(name);

  if (!ok)
  {
    qDebug() << "Trying to reuse existing pipe";
    ok = m_server->removeServer(name);
    if (ok)
    {
      qDebug("Server removed, connecting again");
      ok = m_server->listen(name);
    }
  }

  qDebug() << "Opened" << ok;

  return ok;
}


QString CuteIPCService::serverName() const
{
  return m_server->serverName();
}


void CuteIPCService::newConnection()
{
  QLocalSocket* socket = m_server->nextPendingConnection();
  Q_ASSERT(socket != 0);

  new CuteIPCServiceConnection(socket, this);
}


void CuteIPCService::handleSignalRequest(QString signalSignature)
{
  qDebug() << Q_FUNC_INFO;
  CuteIPCSignalHandler* handler = m_signalHandlers.value(signalSignature);
  if (!handler)
  {
    //create a new signal handler
    qDebug() << "Create a new signal handler for the signature: " << signalSignature;
    handler = new CuteIPCSignalHandler(signalSignature, this);
    m_signalHandlers.insert(signalSignature, handler);

    bool ok = QMetaObject::connect(this,
                                   this->metaObject()->indexOfSignal(
                                       QMetaObject::normalizedSignature(signalSignature.toAscii())),
                                   handler,
                                   handler->metaObject()->indexOfSlot("relaySlot()"));
  }

  CuteIPCServiceConnection* senderConnection = qobject_cast<CuteIPCServiceConnection*> (QObject::sender());
  handler->addListener(senderConnection);
}


void CuteIPCService::removeSignalHandler(QString key)
{
  m_signalHandlers.remove(key);
}
