#ifndef CUTEIPCINTERFACE_P_H
#define CUTEIPCINTERFACE_P_H

// Qt
#include <QObject>
#include <QHash>
class QLocalSocket;

// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMessage_p.h"
class CuteIPCInterfaceConnection;
class CuteIPCSlotHandler;


class CuteIPCInterfacePrivate
{
  Q_DECLARE_PUBLIC(CuteIPCInterface)

  typedef QPair<QObject*,QString> SlotData;

  public:
    CuteIPCInterfacePrivate();
    virtual ~CuteIPCInterfacePrivate();

    QLocalSocket* m_socket;
    CuteIPCInterfaceConnection* m_connection;

    void registerSocket();
    bool sendSynchronousRequest(const QByteArray& request);
    bool checkConnectCorrection(const QString& signal, const QObject* object, const QString& slot);

    void registerConnection(const QString& signalSignature, QObject* reciever, const QString& slotSignature);
    void removeConnection(const QString& signalSignature);
    void _q_invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);

    CuteIPCInterface* q_ptr;
    QHash<QString,SlotData> m_connections;
};

#endif //CUTEIPCINTERFACE_P_H
