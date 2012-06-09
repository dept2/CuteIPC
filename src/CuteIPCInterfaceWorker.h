#ifndef CUTEIPCINTERFACEWORKER_H
#define CUTEIPCINTERFACEWORKER_H

#include <QObject>
#include <QPointer>
class QLocalSocket;

// Local
#include <CuteIPCMessage_p.h>
class CuteIPCInterfaceConnection;
class CuteIPCLoopVector;


class CuteIPCInterfaceWorker : public QObject
{
  Q_OBJECT

  public:
    explicit CuteIPCInterfaceWorker(QObject* parent = 0);
    ~CuteIPCInterfaceWorker();

  signals:
    void invokeRemoteSignal(const QString& methodName, const CuteIPCMessage::Arguments& arguments);
    void setLastError(const QString& error);

    // slot finish signals
    void registerSocketFinished();
    void connectToServerFinished();
    void disconnectFromServerFinished();
    void setReturnedObjectFinished();
    void sendSynchronousRequestFinished();

  public slots:
    void registerSocket();
    void connectToServer(const QString& name, void* successful);
    void disconnectFromServer();

    void sendSynchronousRequest(const QByteArray& request, void* successful,
                                QGenericReturnArgument returnedObject = QGenericReturnArgument());
    void sendSignal(const QByteArray& request);

    void sendCallRequest(const QByteArray& request);

  private:
    QPointer<CuteIPCInterfaceConnection> m_connection;
    QPointer<QLocalSocket> m_socket;
    CuteIPCLoopVector* m_syncCallLoops;
};

#endif // CUTEIPCINTERFACEWORKER_H
