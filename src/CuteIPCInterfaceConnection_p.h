#ifndef CUTEIPCINTERFACECONNECTION_P_H
#define CUTEIPCINTERFACECONNECTION_P_H

// Qt
#include <QObject>
#include <QLocalSocket>

// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMessage_p.h"

class CuteIPCInterfaceConnection: public QObject
{
  Q_OBJECT

  public:
    CuteIPCInterfaceConnection(QLocalSocket* socket, CuteIPCInterface* parent);

    void sendCallRequest(const QByteArray& request);
    void setReturnedObject(QGenericReturnArgument returnedObject);
    QString lastError() const;
    bool lastCallSuccessful() const;

  signals:
    void callFinished();
    void invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);

  public slots:
    void readyRead();
    void errorOccured(QLocalSocket::LocalSocketError);

  private:
    QLocalSocket* m_socket;
    quint32 m_nextBlockSize;
    QByteArray m_block;

    QString m_lastError;
    bool m_lastCallSuccessful;
    QGenericReturnArgument m_returnedObject;

    bool readMessageFromSocket();
};

#endif // CUTEIPCINTERFACECONNECTION_P_H
