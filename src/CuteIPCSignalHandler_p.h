#ifndef CUTEIPCSIGNALHANDLER_P_H
#define CUTEIPCSIGNALHANDLER_P_H

// Qt
#include <QObject>

// Local
class CuteIPCService;
class CuteIPCServiceConnection;


class CuteIPCSignalHandler : public QObject
{
  Q_OBJECT_FAKE

  public:
    explicit CuteIPCSignalHandler(const QString& signature, CuteIPCService* parent = 0);
    ~CuteIPCSignalHandler();

//  public slots:
    void relaySlot(void** args);
    void addListener(CuteIPCServiceConnection* listener);
    void listenerDestroyed(QObject* listener);

  protected:
//  signals:
    void signalCaptured(const QByteArray& data);

  private:
    QString m_signature;
    QList<CuteIPCServiceConnection*> m_listeners;
};

#endif // CUTEIPCSIGNALHANDLER_P_H
