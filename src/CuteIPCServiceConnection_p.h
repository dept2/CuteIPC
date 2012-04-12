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
    ~CuteIPCServiceConnection();

  signals:
    void signalRequest(QString signalSignature);

  public slots:
    void readyRead();
    void errorOccured(QLocalSocket::LocalSocketError);
    void sendSignal(const QByteArray& data);

  private:
    QLocalSocket* m_socket;
    quint32 m_nextBlockSize;
    QByteArray m_block;

    void processMessage();

    void sendResponseMessage(QGenericArgument arg = QGenericArgument());
    void sendErrorMessage(const QString& error);
    void sendResponse(const QByteArray& response);
};


#endif // CUTEIPCSERVICECONNECTION_P_H
