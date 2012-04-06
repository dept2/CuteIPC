//local
#include "CuteIPCMessage_p.h"

//Qt
#include <QString>


CuteIPCMessage::CuteIPCMessage(QString method, Arguments arguments,
                                       QString returnType, CallType callType)
{
  m_method = method;
  m_arguments = arguments;
  m_returnType = returnType;
  m_callType = callType;
}


CuteIPCMessage::CuteIPCMessage(QString method,
                                       QGenericArgument val0, QGenericArgument val1,
                                       QGenericArgument val2, QGenericArgument val3,
                                       QGenericArgument val4, QGenericArgument val5,
                                       QGenericArgument val6, QGenericArgument val7,
                                       QGenericArgument val8, QGenericArgument val9,
                                       QString returnType, bool withConfirm)
{
  m_method = method;
  m_returnType = returnType;

  if (returnType.isEmpty())
    m_callType = withConfirm ? CALL_WITH_CONFIRM : CALL_WITHOUT_CONFIRM;
  else
    m_callType = CALL_WITH_RETURN;

  if (val0.data())
    m_arguments.append(val0);
  if (val1.data())
    m_arguments.append(val1);
  if (val2.data())
    m_arguments.append(val2);
  if (val3.data())
    m_arguments.append(val3);
  if (val4.data())
    m_arguments.append(val4);
  if (val5.data())
    m_arguments.append(val5);
  if (val6.data())
    m_arguments.append(val6);
  if (val7.data())
    m_arguments.append(val7);
  if (val8.data())
    m_arguments.append(val8);
  if (val9.data())
    m_arguments.append(val9);
}

const QString& CuteIPCMessage::method() const
{
  return m_method;
}


const CuteIPCMessage::Arguments& CuteIPCMessage::arguments() const
{
  return m_arguments;
}


const QString& CuteIPCMessage::returnType() const
{
  return m_returnType;
}


const CuteIPCMessage::CallType& CuteIPCMessage::callType() const
{
  return m_callType;
}
