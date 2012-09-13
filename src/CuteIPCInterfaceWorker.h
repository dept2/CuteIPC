#ifndef CUTEIPCINTERFACEWORKER_H
#define CUTEIPCINTERFACEWORKER_H

#include <QObject>
#include <QPointer>
#include <QMultiHash>
class QLocalSocket;

// Local
#include <CuteIPCMessage_p.h>
class CuteIPCInterfaceConnection;
class CuteIPCLoopVector;


class CuteIPCInterfaceWorker : public QObject
{
  Q_OBJECT

  typedef QPair<QObject*,QString> MethodData;

  public:
    explicit CuteIPCInterfaceWorker(QObject* parent = 0);
    ~CuteIPCInterfaceWorker();

  signals:
    void setLastError(const QString& error);

    // slot finish signals
    void registerSocketFinished();
    void connectToServerFinished();
    void disconnectFromServerFinished();

  public slots:
    void registerSocket();
    void connectToServer(const QString& name, void* successful);
    void disconnectFromServer();
    void sendCallRequest(const QByteArray& request);

    void remoteConnect(const QString& signalSignature, void* object, const QString& methodSignature);
    void disconnectSignal(const QString& signalSignature, void* object, const QString& method);
    void removeRemoteConnectionsOfObject(QObject* destroyedObject);
    void invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);

  private:
    void sendRemoteConnectionRequest(const QString& signal);
    void sendSignalDisconnectRequest(const QString& signal);
    void registerConnection(const QString& signalSignature, QObject* reciever, const QString& methodSignature);

    QPointer<CuteIPCInterfaceConnection> m_connection;
    QPointer<QLocalSocket> m_socket;
    QMultiHash<QString,MethodData> m_connections;
};

#endif // CUTEIPCINTERFACEWORKER_H
