#ifndef CUTEIPCSERVICE_H
#define CUTEIPCSERVICE_H

// Qt
#include <QObject>

// Local
class CuteIPCServicePrivate;

class CuteIPCService : public QObject
{
  Q_OBJECT

  public:
    explicit CuteIPCService(QObject* parent = 0);
    ~CuteIPCService();

    bool listen(const QString& name = QString(), QObject* subject = 0);
    void close();
    QString serverName() const;

  protected:
    CuteIPCServicePrivate* const d_ptr;
    CuteIPCService(CuteIPCServicePrivate& dd, QObject* parent);

  private:
    Q_DECLARE_PRIVATE(CuteIPCService)

    Q_PRIVATE_SLOT(d_func(),void _q_newConnection())
    Q_PRIVATE_SLOT(d_func(),void _q_handleSignalRequest(QString, QObject*))
    Q_PRIVATE_SLOT(d_func(),void _q_handleSignalDisconnect(QString, QObject*))
    Q_PRIVATE_SLOT(d_func(),void _q_removeSignalHandler(QString))
};

#endif // CUTEIPCSERVICE_H
