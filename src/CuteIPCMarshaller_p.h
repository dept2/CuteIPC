#ifndef CUTEIPCMARSHALLER_P_H
#define CUTEIPCMARSHALLER_P_H

// Qt
#include <QString>
#include <QByteArray>
#include <QPair>
#include <QGenericArgument>

// local
#include "CuteIPCMessage_p.h"

class CuteIPCMarshaller
{
  public:
    static QByteArray marshallMessage(const CuteIPCMessage& message);
    static CuteIPCMessage demarshallMessage(QByteArray& call);

    static CuteIPCMessage::MessageType demarshallMessageType(QByteArray& message);
    static void demarshallResponse(QByteArray& call, QGenericReturnArgument arg);

    static void freeArguments(const CuteIPCMessage::Arguments&);

  private:
    static bool marshallArgumentToStream(QGenericArgument value, QDataStream& stream);
    static QGenericArgument demarshallArgumentFromStream(bool& ok, QDataStream& stream);

};

#endif // CUTEIPCMARSHALLER_P_H
