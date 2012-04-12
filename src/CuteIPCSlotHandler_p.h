#ifndef CUTEIPCSLOTHANDLER_P_H
#define CUTEIPCSLOTHANDLER_P_H

// Qt
#include <QObject>
#include <QPair>
#include <QString>
#include <QHash>

// Local
#include <CuteIPCMessage_p.h>
class CuteIPCInterface;
class CuteIPCInterfaceConnection;


class CuteIPCSlotHandler : public QObject
{
  Q_OBJECT

  typedef QPair<QObject*,QString> SlotData;

  public:
    explicit CuteIPCSlotHandler(CuteIPCInterface* parent = 0);
    ~CuteIPCSlotHandler();

  public slots:
    void invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);
    void registerConnection(const QString& signalSignature, QObject* reciever, const QString& slotSignature);
    void removeConnection(const QString& signalSignature);

  private:
    QHash<QString,SlotData> m_connections;
};

#endif // CUTEIPCSLOTHANDLER_P_H
