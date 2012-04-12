#ifndef CUTEIPCINTERFACECONNECTION_P_H
#define CUTEIPCINTERFACECONNECTION_P_H

// Qt
#include <QObject>
#include <QLocalSocket>

// Local
#include "CuteIPCInterface.h"

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
