// Local
#include "CuteIPCSlotHandler_p.h"
#include "CuteIPCInterface.h"

// Qt
#include <QDebug>
#include <QObject>


CuteIPCSlotHandler::CuteIPCSlotHandler(CuteIPCInterface* parent)
  : QObject(parent)
{
  qDebug() << Q_FUNC_INFO;
}


CuteIPCSlotHandler::~CuteIPCSlotHandler()
{
  qDebug() << Q_FUNC_INFO;
}


void CuteIPCSlotHandler::registerConnection(const QString& signalSignature,
                                            QObject *reciever,
                                            const QString& slotSignature)
{
  m_connections.insert(signalSignature, SlotData(reciever, slotSignature));
}


void CuteIPCSlotHandler::removeConnection(const QString& signalSignature)
{
  m_connections.remove(signalSignature);
}


void CuteIPCSlotHandler::invokeRemoteSignal(const QString& signalSignature,
                                            const CuteIPCMessage::Arguments &arguments)
{
  SlotData data = m_connections.value(signalSignature);
  if (!data.first)
    return;

  qDebug() << "SIGNAL: Trying to invoke slot";

  QString methodName = data.second;
  methodName = methodName.left(methodName.indexOf("("));

  CuteIPCMessage::Arguments args = arguments;
  while (args.size() < 10)
    args.append(QGenericArgument());

  bool successfulInvoke = QMetaObject::invokeMethod(data.first, methodName.toAscii(),
          args.at(0), args.at(1), args.at(2),
          args.at(3), args.at(4), args.at(5),
          args.at(6), args.at(7), args.at(8),
          args.at(9));

  //TODO: need to cleanup memory!
  qDebug() << "SIGNAL: invoke slot:" << successfulInvoke;
}
