#ifndef CUTEIPCINTERFACECONNECTION_P_H
#define CUTEIPCINTERFACECONNECTION_P_H

// Qt
#include <QObject>
#include <QLocalSocket>

// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMessage_p.h"

class CuteIPCInterfaceConnection : public QObject
{
  Q_OBJECT

  public:
    CuteIPCInterfaceConnection(QLocalSocket* socket, QObject* parent);

    void sendCallRequest(const QByteArray& request);
    void setReturnedObject(QGenericReturnArgument returnedObject);
    bool lastCallSuccessful() const;

  signals:
    void callFinished();
    void invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);
    void errorOccured(const QString&);

  public slots:
    void readyRead();
    void errorOccured(QLocalSocket::LocalSocketError);

  private:
    QLocalSocket* m_socket;
    quint32 m_nextBlockSize;
    QByteArray m_block;

    bool m_lastCallSuccessful;
    QList<QGenericReturnArgument> m_returnedObjects;

    bool readMessageFromSocket();
};

#endif // CUTEIPCINTERFACECONNECTION_P_H
