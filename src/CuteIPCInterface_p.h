#ifndef CUTEIPCINTERFACE_P_H
#define CUTEIPCINTERFACE_P_H

// Qt
#include <QObject>
#include <QMultiHash>
#include <QPointer>
class QLocalSocket;

// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMessage_p.h"
class CuteIPCInterfaceConnection;
class CuteIPCSignalHandler;
class CuteIPCInterfaceWorker;
class CuteIPCLoopVector;


class CuteIPCInterfacePrivate
{
  Q_DECLARE_PUBLIC(CuteIPCInterface)

  typedef QPair<QObject*,QString> MethodData;

  public:
    CuteIPCInterfacePrivate();
    virtual ~CuteIPCInterfacePrivate();

    void registerSocket();

    bool checkConnectCorrection(const QString& signal, const QString& method);
    void sendRemoteConnectionRequest(const QString& signal);
    void sendSignalDisconnectRequest(const QString& signal);
    bool checkRemoteSlotExistance(const QString& slot);
    bool sendSynchronousRequest(const QByteArray& request, QGenericReturnArgument returnedObject = QGenericReturnArgument());

    void registerConnection(const QString& signalSignature, QObject* reciever, const QString& methodSignature);
    void removeConnection(const QString& signalSignature);

    void handleLocalSignalRequest(QObject* localObject, const QString& signalSignature, const QString& slotSignature);

    void _q_sendAsynchronousRequest(const QByteArray& request);
    void _q_invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);
    void _q_removeSignalHandlersOfObject(QObject*);
    void _q_removeRemoteConnectionsOfObject(QObject*);
    void _q_setLastError(QString); //TODO: !!!!!!

    CuteIPCInterface* q_ptr;
    QMultiHash<QString,MethodData> m_connections;
    QMultiHash<MethodData, CuteIPCSignalHandler*> m_localSignalHandlers;
    QString m_lastError;
    QThread* m_workerThread;
    CuteIPCInterfaceWorker* m_worker;
    QString m_serverName;
};

#endif //CUTEIPCINTERFACE_P_H
