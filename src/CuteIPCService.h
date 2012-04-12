#ifndef CUTEIPCSERVICE_H
#define CUTEIPCSERVICE_H

// Qt
#include <QObject>
#include <QHash>
#include <QMap>
class QLocalServer;

// local
class CuteIPCSignalHandler;

class CuteIPCService : public QObject
{
  Q_OBJECT

  public:
    CuteIPCService(QObject* parent = 0);
    ~CuteIPCService();

    bool listen(const QString& name = QString());

    QString serverName() const;

  public slots:
    void handleSignalRequest(QString signalSignature);
    void removeSignalHandler(QString);

  private slots:
    void newConnection();

  private:
    QLocalServer* m_server;
    QHash<QString, CuteIPCSignalHandler* > m_signalHandlers;
};

#endif // CUTEIPCSERVICE_H
