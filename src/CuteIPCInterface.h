#ifndef CUTEIPCINTERFACE_H
#define CUTEIPCINTERFACE_H

// Qt
#include <QObject>
class QLocalSocket;

// Local
class CuteIPCInterfaceConnection;


class CuteIPCInterface : public QObject
{
  Q_OBJECT

  public:
    CuteIPCInterface(QObject* parent = 0);
    bool connectToServer(const QString& name);

    // FIXME: temporary, will be properly done later
    bool connectRemoteSignal(const char* signal);

    bool call(const QString& method, QGenericReturnArgument ret, QGenericArgument val0 = QGenericArgument(),
              QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
              QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
              QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
              QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
              QGenericArgument val9 = QGenericArgument());

    bool call(const QString& method, QGenericArgument val0 = QGenericArgument(),
              QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
              QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
              QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
              QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
              QGenericArgument val9 = QGenericArgument());

    void callAsynchronous(const QString& method, QGenericArgument val0 = QGenericArgument(),
              QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
              QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
              QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
              QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
              QGenericArgument val9 = QGenericArgument());

    QString lastError() const;

  private:
    QLocalSocket* m_socket;
    CuteIPCInterfaceConnection* m_connection;

    bool sendSynchronousRequest(const QByteArray& request);
};

#endif // CUTEIPCINTERFACE_H
