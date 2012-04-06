#ifndef CUTEIPCMARSHALLER_P_H
#define CUTEIPCMARSHALLER_P_H

// Qt
#include <QString>
#include <QByteArray>
#include <QPair>
#include <QGenericArgument>

// local
#include "CuteIPCMessage_p.h"

enum MessageType
{
  MESSAGE_CALL,
  MESSAGE_RETURN,
  MESSAGE_STATUS
};

class CuteIPCMarshaller
{
  public:
    typedef QPair<bool, QString> Status;

    static MessageType demarshallHeader(QByteArray& message);

    static QByteArray marshallCall(const CuteIPCMessage& message);
    static CuteIPCMessage demarshallCall(QByteArray& call);

    static QByteArray marshallReturnedValue(QGenericArgument value);
    static void demarshallReturnedValue(QByteArray& value, QGenericReturnArgument arg);

    static QByteArray marshallStatusMessage(Status status);
    static Status demarshallStatusMessage(QByteArray& message);

    static void freeArguments(const CuteIPCMessage::Arguments&);

  private:
    //service static methods
    static void marshallHeaderToStream(MessageType type, QDataStream& stream);
    static MessageType demarshallHeaderFromStream(QDataStream& stream);

    static bool marshallArgumentToStream(QGenericArgument value, QDataStream& stream);
    static QGenericArgument demarshallArgumentFromStream(bool& ok, QDataStream& stream);

};

#endif // CUTEIPCMARSHALLER_P_H
