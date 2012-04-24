#ifndef CUTEIPCSERVICE_P_H
#define CUTEIPCSERVICE_P_H

// Qt
#include <QObject>
#include <QHash>
class QLocalServer;

// Local
#include "CuteIPCService.h"
class CuteIPCSignalHandler;

class CuteIPCServicePrivate
{
  Q_DECLARE_PUBLIC(CuteIPCService)

  public:
    CuteIPCServicePrivate();
    virtual ~CuteIPCServicePrivate();

    void registerServer();

    void _q_newConnection();
    void _q_handleSignalRequest(QString, QObject*);
    void _q_handleSignalDisconnect(QString, QObject*);
    void _q_removeSignalHandler(QString);

    QLocalServer* m_server;
    QHash<QString, CuteIPCSignalHandler*> m_signalHandlers;

    CuteIPCService* q_ptr;
};

#endif // CUTEIPCSERVICE_P_H
