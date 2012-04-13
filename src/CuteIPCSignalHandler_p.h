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
    explicit CuteIPCSignalHandler(const QString& signature, QObject* parent = 0);
    ~CuteIPCSignalHandler();

//  public slots:
    void relaySlot(void**);
    void addListener(CuteIPCServiceConnection* listener);
    void listenerDestroyed(QObject* listener);

  protected:
//  signals:
    void signalCaptured(const QByteArray& data);
    void destroyed(QString signature);

  private:
    QString m_signature;
    QList<CuteIPCServiceConnection*> m_listeners;
};

#endif // CUTEIPCSIGNALHANDLER_P_H
