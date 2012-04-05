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

  signals:
    void callFinished();

  public slots:
    void readyRead();
    void errorOccured(QLocalSocket::LocalSocketError);

  private:
    QLocalSocket* m_socket;
    quint32 m_nextBlockSize;
    QByteArray m_block;

    QGenericReturnArgument m_returnedObject;
};

#endif // CUTEIPCINTERFACECONNECTION_P_H
