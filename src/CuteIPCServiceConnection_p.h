#ifndef CUTEIPCSERVICECONNECTION_P_H
#define CUTEIPCSERVICECONNECTION_P_H

// Qt
#include <QObject>
class QLocalSocket;

// Local
#include "CuteIPCService.h"


class CuteIPCServiceConnection : public QObject
{
  Q_OBJECT

  public:
    CuteIPCServiceConnection(QLocalSocket* socket, CuteIPCService* parent);

  public slots:
    void readyRead();

  private:
    QLocalSocket* m_socket;
    quint32 m_nextBlockSize;
    QByteArray m_block;
};

#endif // CUTEIPCSERVICECONNECTION_P_H
