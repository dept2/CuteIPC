#ifndef CUTEIPCSERVICE_H
#define CUTEIPCSERVICE_H

// Qt
#include <QObject>
class QLocalServer;


class CuteIPCService : public QObject
{
  Q_OBJECT

  public:
    CuteIPCService(QObject* parent = 0);
    ~CuteIPCService();

    bool listen(const QString& name = QString());

    QString serverName() const;

  private slots:
    void newConnection();

  private:
    QLocalServer* m_server;
};

#endif // CUTEIPCSERVICE_H
