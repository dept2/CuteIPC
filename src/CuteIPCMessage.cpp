//local
#include "CuteIPCMessage_p.h"

//Qt
#include <QString>


CuteIPCMessage::CuteIPCMessage(MessageType type)
{
  this->m_type = type;
}


CuteIPCMessage::MessageType CuteIPCMessage::type() const
{
  return m_type;
}


CuteIPCMessageStatus::CuteIPCMessageStatus(bool status, QString message)
    : CuteIPCMessage(MESSAGE_STATUS)
{
  m_status = status;
  m_message = message;
}


bool CuteIPCMessageStatus::status() const
{
  return m_status;
}


QString CuteIPCMessageStatus::message() const
{
  return m_message;
}


CuteIPCMessageCall::CuteIPCMessageCall(QString method, Arguments arguments,
                                       QString returnType, CallType callType)
    : CuteIPCMessage(MESSAGE_CALL)
{
  m_method = method;
  m_arguments = arguments;
  m_returnType = returnType;
  m_callType = callType;
}


CuteIPCMessageCall::CuteIPCMessageCall(QString method,
                                       QGenericArgument val0, QGenericArgument val1,
                                       QGenericArgument val2, QGenericArgument val3,
                                       QGenericArgument val4, QGenericArgument val5,
                                       QGenericArgument val6, QGenericArgument val7,
                                       QGenericArgument val8, QGenericArgument val9,
                                       QString returnType, bool withConfirm)
    : CuteIPCMessage(MESSAGE_CALL)
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

QString CuteIPCMessageCall::method() const
{
  return m_method;
}


CuteIPCMessageCall::Arguments CuteIPCMessageCall::arguments() const
{
  return m_arguments;
}


QString CuteIPCMessageCall::returnType() const
{
  return m_returnType;
}


CuteIPCMessageCall::CallType CuteIPCMessageCall::callType() const
{
  return m_callType;
}


CuteIPCMessageReturn::CuteIPCMessageReturn(QGenericArgument value)
    : CuteIPCMessage(MESSAGE_RETURN)
{
  m_value = value;
}


QGenericArgument CuteIPCMessageReturn::value() const
{
  return m_value;
}
