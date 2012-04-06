#ifndef CUTEIPCMESSAGE_P_H
#define CUTEIPCMESSAGE_P_H

// Qt
#include <QObject>


class CuteIPCMessage
{
  public:
    typedef QList<QGenericArgument> Arguments;

    enum CallType
    {
      CALL_WITH_RETURN,
      CALL_WITH_CONFIRM,
      CALL_WITHOUT_CONFIRM
    };

    CuteIPCMessage(QString method, Arguments arguments, QString returnType, CallType callType);

    CuteIPCMessage(QString method,
        QGenericArgument val0 = QGenericArgument(),
        QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
        QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
        QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
        QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
        QGenericArgument val9 = QGenericArgument(), QString returnType = QString(), bool withConfirm = true);

    const QString& method() const;
    const Arguments& arguments() const;
    const QString& returnType() const;
    const CallType& callType() const;

  private:
    QString m_method;
    Arguments m_arguments;
    QString m_returnType;
    CallType m_callType;
};


#endif // CUTEIPCMESSAGE_P_H
