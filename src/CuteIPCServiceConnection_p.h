#ifndef CUTEIPCSERVICECONNECTION_P_H
#define CUTEIPCSERVICECONNECTION_P_H

// Qt
#include <QObject>
#include <QLocalSocket>

// Local
#include "CuteIPCService.h"


class CuteIPCServiceConnection : public QObject
{
  Q_OBJECT

  public:
    CuteIPCServiceConnection(QLocalSocket* socket, CuteIPCService* parent);

  public slots:
    void readyRead();
    void errorOccured(QLocalSocket::LocalSocketError);

  private:
    QLocalSocket* m_socket;
    quint32 m_nextBlockSize;
    QByteArray m_block;

    void makeCall();
    void sendReturnedValue(QGenericArgument arg);
    void sendConfirm();
};


#endif // CUTEIPCSERVICECONNECTION_P_H
