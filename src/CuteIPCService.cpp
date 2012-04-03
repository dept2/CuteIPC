// Local
#include "CuteIPCService.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCServiceConnection_p.h"

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
