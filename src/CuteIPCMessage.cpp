//local
#include "CuteIPCMessage_p.h"

//Qt
#include <QString>


CuteIPCMessage::CuteIPCMessage(MessageType type, const QString& method,
                               const Arguments& arguments, QString returnType)
{
  m_method = method;
  m_arguments = arguments;
  m_messageType = type;
  m_returnType = returnType;
}


CuteIPCMessage::CuteIPCMessage(MessageType type, QString method,
                               QGenericArgument val0, QGenericArgument val1,
                               QGenericArgument val2, QGenericArgument val3,
                               QGenericArgument val4, QGenericArgument val5,
                               QGenericArgument val6, QGenericArgument val7,
                               QGenericArgument val8, QGenericArgument val9,
                               QString returnType)
{
  m_messageType = type;

  m_method = method;

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

  m_returnType = returnType;
}


const QString& CuteIPCMessage::method() const
{
  return m_method;
}


const CuteIPCMessage::Arguments& CuteIPCMessage::arguments() const
{
  return m_arguments;
}


const CuteIPCMessage::MessageType& CuteIPCMessage::messageType() const
{
  return m_messageType;
}


const QString& CuteIPCMessage::returnType() const
{
  return m_returnType;
}
