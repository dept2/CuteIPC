#ifndef CUTEIPCINTERFACE_H
#define CUTEIPCINTERFACE_H

// Qt
#include <QObject>
class QHostAddress;

// Local
class CuteIPCInterfacePrivate;


class CuteIPCInterface : public QObject
{
  Q_OBJECT

  public:
    CuteIPCInterface(QObject* parent = 0);
    ~CuteIPCInterface();

    bool connectToServer(const QString& name);
    bool connectToServer(const QHostAddress& host, quint16 port);

    void disconnectFromServer();

    bool remoteConnect(const char* signal, QObject* object, const char* method);
    bool remoteSlotConnect(QObject* localObject, const char* signal, const char* remoteSlot);

    bool disconnectSignal(const char* signal, QObject* object, const char* method);
    bool disconnectSlot(QObject* localObject, const char* signal, const char* remoteSlot);

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


    void callNoReply(const QString& method, QGenericArgument val0 = QGenericArgument(),
              QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
              QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
              QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
              QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
              QGenericArgument val9 = QGenericArgument());

    QString lastError() const;

  protected:
    CuteIPCInterfacePrivate* const d_ptr;
    CuteIPCInterface(CuteIPCInterfacePrivate& dd, QObject* parent);

  private:
    Q_DECLARE_PRIVATE(CuteIPCInterface)
    Q_PRIVATE_SLOT(d_func(),void _q_sendAsynchronousRequest(QByteArray))
    Q_PRIVATE_SLOT(d_func(),void _q_removeSignalHandlersOfObject(QObject*))
    Q_PRIVATE_SLOT(d_func(), void _q_setLastError(QString))
    Q_PRIVATE_SLOT(d_func(), void _q_invokeRemoteSignal(QString, CuteIPCMessage::Arguments))
    Q_PRIVATE_SLOT(d_func(), void _q_removeRemoteConnectionsOfObject(QObject*))
};

#endif // CUTEIPCINTERFACE_H
