// Local
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCMessage_p.h"

// Qt
#include <QDataStream>
#include <QPair>
#include <QMetaType>
#include <QDebug>


MessageType CuteIPCMarshaller::demarshallHeader(QByteArray message)
{
  QDataStream stream(&message, QIODevice::ReadOnly);
  return demarshallHeaderFromStream(stream);
}


QByteArray CuteIPCMarshaller::marshallCall(const QString& method,
    QGenericArgument val0, QGenericArgument val1,
    QGenericArgument val2, QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
    QGenericArgument val7, QGenericArgument val8, QGenericArgument val9, QString retType, bool withConfirm)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);
  marshallHeaderToStream(MESSAGE_CALL, stream);

  // Method name
  stream << method;

  if (retType.isEmpty())
  {
    stream << (withConfirm ? CALL_WITH_CONFIRM : CALL_WITHOUT_CONFIRM);
  }
  else
  {
    stream << CALL_WITH_RETURN;
    stream << retType;
  }

  // Parse input args
  Arguments args;
  if (val0.data())
    args.append(val0);
  if (val1.data())
    args.append(val1);
  if (val2.data())
    args.append(val2);
  if (val3.data())
    args.append(val3);
  if (val4.data())
    args.append(val4);
  if (val5.data())
    args.append(val5);
  if (val6.data())
    args.append(val6);
  if (val7.data())
    args.append(val7);
  if (val8.data())
    args.append(val8);
  if (val9.data())
    args.append(val9);

  // Write args to stream
  stream << args.size(); // Argument count

  bool successfullyMarshalled;
  foreach (const QGenericArgument& arg, args)
  {
    successfullyMarshalled = marshallArgumentToStream(arg, stream);
    if (!successfullyMarshalled)
      return QByteArray();
  }

  return result;
}


QByteArray CuteIPCMarshaller::marshallCall(const CuteIPCMessageCall &message)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);
  marshallHeaderToStream(message.type(), stream);

  stream << message.method();
  stream << message.callType();
  stream << message.returnType();

  bool successfullyMarshalled;
  foreach (const QGenericArgument& arg, message.arguments())
  {
    successfullyMarshalled = marshallArgumentToStream(arg, stream);
    if (!successfullyMarshalled)
      return QByteArray();
  }

  return result;
}

CuteIPCMarshaller::Call CuteIPCMarshaller::demarshallCall(QByteArray call)
{
  QDataStream stream(&call, QIODevice::ReadOnly);
  demarshallHeaderFromStream(stream);

  // Method
  QString method;
  stream >> method;

  CallType calltype;
  QString retType;

  int buffer;
  stream >> buffer;
  calltype = CallType(buffer);

  if (calltype == CALL_WITH_RETURN)
    stream >> retType;

  // Arguments count
  int argc = 0;
  stream >> argc;
  Q_ASSERT(argc <= 10);

  Arguments args;

  for (int i = 0; i < argc; ++i)
  {
    bool ok;
    QGenericArgument argument = demarshallArgumentFromStream(ok, stream);
    if (!ok)
    {
      qWarning() << "Failed to deserialize argument" << i;
      break;
    }
    args.append(argument);
  }

  // Fill empty args
  while (args.size() < 10)
    args.append(QGenericArgument());

  return Call(method, args, retType, calltype);
}


QByteArray CuteIPCMarshaller::marshallReturnedValue(QGenericArgument returnedValue)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);
  marshallHeaderToStream(MESSAGE_RETURN, stream);

  bool successfullyMarshalled = marshallArgumentToStream(returnedValue, stream);
  if (!successfullyMarshalled)
    return QByteArray(); //TODO: send error status
  return result;
}


void CuteIPCMarshaller::demarshallReturnedValue(QByteArray value, QGenericReturnArgument arg)
{
  QDataStream stream(&value, QIODevice::ReadOnly);
  demarshallHeaderFromStream(stream);

  // Load type
  QString typeName;
  stream >> typeName;

  // Check type
  int type = QMetaType::type(typeName.toLatin1());
  if (type == 0)
  {
    qWarning() << "Unsupported type of argument " << ":" << typeName;
  }
  if (type != QMetaType::type(arg.name()))
  {
    qWarning() << "Type doesn't match:" << typeName << "Expected:" << arg.name();
  }

  // Read argument data from stream
//  void* data = QMetaType::construct(type);

  bool dataLoaded = QMetaType::load(stream, type, arg.data());
  if (!dataLoaded)
  {
    qWarning() << "Failed to deserialize argument value" << "of type" << typeName;
  }
}


QByteArray CuteIPCMarshaller::marshallStatusMessage(Status status)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);
  marshallHeaderToStream(MESSAGE_STATUS, stream);

  stream << status.first;
  stream << status.second;
  return result;
}


CuteIPCMarshaller::Status CuteIPCMarshaller::demarshallStatusMessage(QByteArray message)
{
  QDataStream stream(&message, QIODevice::ReadOnly);
  demarshallHeaderFromStream(stream);

  Status status;
  stream >> status.first;
  stream >> status.second;
  return status;
}


void CuteIPCMarshaller::marshallHeaderToStream(MessageType type, QDataStream& stream)
{
  stream << type;
}

MessageType CuteIPCMarshaller::demarshallHeaderFromStream(QDataStream& stream)
{
  int buffer;
  stream >> buffer;
  return MessageType(buffer);
}


bool CuteIPCMarshaller::marshallArgumentToStream(QGenericArgument returnedValue, QDataStream& stream)
{
  // Detect and check type
  int type = QMetaType::type(returnedValue.name());
    if (type == 0)
    {
      qWarning() << "Type" << returnedValue.name() << "have not been registered in Qt metaobject system";
      return false;
    }

    stream << QString::fromLatin1(returnedValue.name());
    bool ok = QMetaType::save(stream, type, returnedValue.data());
    if (!ok)
    {
      qWarning() << "Failed to serialize" << returnedValue.name()
                 << "to data stream. Call qRegisterMetaTypeStreamOperators to"
                    " register stream operators for this metatype";
      return false;
    }

  return true;
}


QGenericArgument CuteIPCMarshaller::demarshallArgumentFromStream(bool& ok, QDataStream& stream)
{
  // Load type
  QString typeName;
  stream >> typeName;

  // Check type
  int type = QMetaType::type(typeName.toLatin1());
  if (type == 0)
  {
    qWarning() << "Unsupported type of argument " << ":" << typeName;
    ok = false;
    return QGenericArgument();
  }

  // Read argument data from stream
  void* data = QMetaType::construct(type);
  bool dataLoaded = QMetaType::load(stream, type, data);
  if (!dataLoaded)
  {
    qWarning() << "Failed to deserialize argument value" << "of type" << typeName;
    QMetaType::destroy(type, data);
    ok = false;
    return QGenericArgument(); //TODO!: need to discuss
  }

  ok = true;
  return QGenericArgument(qstrdup(typeName.toLatin1()), data);
}


void CuteIPCMarshaller::freeArguments(const CuteIPCMarshaller::Arguments& args)
{
  // Free allocated memory
  for (int i = 0; i < args.size(); ++i)
  {
    const QGenericArgument& arg = args.at(i);
    if (arg.data() == 0)
      continue;

    QMetaType::destroy(QMetaType::type(arg.name()), arg.data());
    delete[] arg.name();
  }
}


CuteIPCMarshaller::Call::Call(QString method, Arguments arguments, QString retType, CallType calltype)
{
  this->first = method;
  this->second = arguments;
  this->retType = retType;
  this->calltype = calltype;
}
