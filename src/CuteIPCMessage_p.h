#ifndef CUTEIPCMESSAGE_P_H
#define CUTEIPCMESSAGE_P_H

// Qt
#include <QObject>

//Local
#include "CuteIPCMarshaller_p.h"

class CuteIPCMessage
{
  public:

  enum MessageType
  {
    MESSAGE_CALL,
    MESSAGE_RETURN,
    MESSAGE_STATUS
  };

  explicit CuteIPCMessage(MessageType type);
  virtual ~CuteIPCMessage() {}
  MessageType type() const;


private:
  MessageType m_type;
};


class CuteIPCMessageCall : public CuteIPCMessage
{
  public:
    typedef QList<QGenericArgument> Arguments;

    enum CallType
    {
      CALL_WITH_RETURN,
      CALL_WITH_CONFIRM,
      CALL_WITHOUT_CONFIRM
    };

    CuteIPCMessageCall(QString method, Arguments arguments, QString returnType, CallType callType);

    CuteIPCMessageCall(QString method,
        QGenericArgument val0 = QGenericArgument(),
        QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
        QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
        QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
        QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
        QGenericArgument val9 = QGenericArgument(), QString returnType = QString(), bool withConfirm = true);

    QString method() const;
    Arguments arguments() const;
    QString returnType() const;
    CallType callType() const;

  private:
    bool m_method;
    Arguments m_arguments;
    QString m_returnType;
    CallType m_callType;
};


class CuteIPCMessageStatus : public CuteIPCMessage
{
  public:
    CuteIPCMessageStatus(bool status, QString message);

    bool status() const;
    QString message() const;

  private:
    bool m_status;
    QString m_message;
};


class CuteIPCMessageReturn : public CuteIPCMessage
{
  public:
    CuteIPCMessageReturn(QGenericArgument value);

    QGenericArgument value() const;

  private:
    QGenericArgument m_value;
};

#endif // CUTEIPCMESSAGE_P_H
