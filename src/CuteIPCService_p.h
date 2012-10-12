#ifndef CUTEIPCSERVICE_P_H
#define CUTEIPCSERVICE_P_H

// Qt
#include <QObject>
#include <QHash>
class QLocalServer;

// Local
#include "CuteIPCService.h"
class CuteIPCSignalHandler;
class CuteIPCServiceConnection;

class CuteIPCServicePrivate
{
  Q_DECLARE_PUBLIC(CuteIPCService)

  public:
    CuteIPCServicePrivate();
    virtual ~CuteIPCServicePrivate();

    void registerServer();

    void _q_newConnection();
    void _q_handleSignalRequest(const QString& signature, const QString& connectionId, QObject*);
    void _q_initializeConnection(QString, QObject*);
    void _q_connectionDestroyed(QObject*);
    void _q_handleSignalDisconnect(const QString& signature, const QString& connectionId, QObject*);
    void _q_removeSignalHandler(QString);

    QLocalServer* m_server;
    QHash<QString, CuteIPCSignalHandler*> m_signalHandlers;
    QHash<QString, QObject*> m_longLivedConnections;
    QObject* m_subject;

    CuteIPCService* q_ptr;
};

#endif // CUTEIPCSERVICE_P_H
