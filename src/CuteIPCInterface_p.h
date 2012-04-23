#ifndef CUTEIPCINTERFACE_P_H
#define CUTEIPCINTERFACE_P_H

// Qt
#include <QObject>
#include <QMultiHash>
class QLocalSocket;

// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMessage_p.h"
class CuteIPCInterfaceConnection;
class CuteIPCSignalHandler;


class CuteIPCInterfacePrivate
{
  Q_DECLARE_PUBLIC(CuteIPCInterface)

  typedef QPair<QObject*,QString> MethodData;

  public:
    CuteIPCInterfacePrivate();
    virtual ~CuteIPCInterfacePrivate();

    QLocalSocket* m_socket;
    CuteIPCInterfaceConnection* m_connection;

    void registerSocket();

    bool checkConnectCorrection(const QString& signal, const QString& slot);
    bool sendRemoteConnectionRequest(const QString& signal);
    bool sendSynchronousRequest(const QByteArray& request);

    void registerConnection(const QString& signalSignature, QObject* reciever, const QString& slotSignature);
    void removeConnection(const QString& signalSignature);

    void handleLocalSignalRequest(QObject* localObject, const QString& signalSignature, const QString& slotSignature);

    void _q_sendSignal(const QByteArray& request);
    void _q_invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);
    void _q_removeSignalHandlersOfObject(QObject*);
    void _q_removeRemoteConnectionsOfObject(QObject*);
    void _q_setLastError(QString);

    CuteIPCInterface* q_ptr;
    QMultiHash<QString,MethodData> m_connections;
    QHash<MethodData, CuteIPCSignalHandler*> m_localSignalHandlers;
    QString m_lastError;
};

#endif //CUTEIPCINTERFACE_P_H
