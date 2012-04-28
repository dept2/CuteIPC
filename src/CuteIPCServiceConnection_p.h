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
    void setSubject(QObject* subject);

  signals:
    void signalRequest(QString signalSignature, QObject* sender);
    void signalDisconnectRequest(QString signalSignature, QObject* sender);

  public slots:
    void readyRead();
    void errorOccured(QLocalSocket::LocalSocketError);
    void sendSignal(const QByteArray& data);

    void sendErrorMessage(const QString& error);
    void sendResponseMessage(const QString& method, QGenericArgument arg = QGenericArgument());

  private:
    QLocalSocket* m_socket;
    quint32 m_nextBlockSize;
    QByteArray m_block;
    QObject* m_subject;

    void processMessage();
    bool readMessageFromSocket();

    void sendResponse(const QByteArray& response);
};


#endif // CUTEIPCSERVICECONNECTION_P_H
