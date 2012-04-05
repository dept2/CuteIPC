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
    typedef QList<QGenericArgument> Arguments;
    typedef QPair<bool, QString> Status;

    enum CallType
    {
      CALL_WITH_RETURN,
      CALL_WITH_CONFIRM,
      CALL_WITHOUT_CONFIRM
    };

    struct Call
    {
      QString first;
      Arguments second;
      QString retType;
      CallType calltype;

      Call(QString method, Arguments arguments, QString retType, CallType calltype);
    };

    static MessageType demarshallHeader(QByteArray message);

    static QByteArray marshallCall(const QString& method,
        QGenericArgument val0 = QGenericArgument(),
        QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
        QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
        QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
        QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
        QGenericArgument val9 = QGenericArgument(), QString retType = QString(), bool withConfirm = true);

    static QByteArray marshallCall(const CuteIPCMessageCall& message);

    static Call demarshallCall(QByteArray call);

    static QByteArray marshallReturnedValue(QGenericArgument value);
    static void demarshallReturnedValue(QByteArray value, QGenericReturnArgument arg);

    static QByteArray marshallStatusMessage(Status status);
    static Status demarshallStatusMessage(QByteArray message);

    static void freeArguments(const Arguments&);

  private:
    //service static methods
    static void marshallHeaderToStream(MessageType type, QDataStream& stream);
    static MessageType demarshallHeaderFromStream(QDataStream& stream);

    static bool marshallArgumentToStream(QGenericArgument value, QDataStream& stream);
    static QGenericArgument demarshallArgumentFromStream(bool& ok, QDataStream& stream);

};

#endif // CUTEIPCMARSHALLER_P_H
