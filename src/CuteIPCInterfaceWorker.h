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

  public slots:
    void registerSocket();
    void connectToServer(const QString& name, void* successful);
    void disconnectFromServer();
    void sendCallRequest(const QByteArray& request);

  private:
    QPointer<CuteIPCInterfaceConnection> m_connection;
    QPointer<QLocalSocket> m_socket;
};

#endif // CUTEIPCINTERFACEWORKER_H
