// Local
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCMessage_p.h"

// Qt
#include <QDataStream>
#include <QPair>
#include <QMetaType>
#include <QDebug>


QByteArray CuteIPCMarshaller::marshallMessage(const CuteIPCMessage& message)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);

  stream << message.messageType();
  stream << message.method();
  stream << message.returnType();
  stream << message.arguments().size();

  bool successfullyMarshalled;
  foreach (const QGenericArgument& arg, message.arguments())
  {
    successfullyMarshalled = marshallArgumentToStream(arg, stream);
    if (!successfullyMarshalled)
      return QByteArray();
  }

  return result;
}


CuteIPCMessage CuteIPCMarshaller::demarshallMessage(QByteArray& call)
{
  QDataStream stream(&call, QIODevice::ReadOnly);

  // Call type
  CuteIPCMessage::MessageType type;
  int buffer;
  stream >> buffer;
  type = CuteIPCMessage::MessageType(buffer);

  // Method
  QString method;
  stream >> method;

  QString returnType;
  stream >> returnType;

  // Arguments
  int argc = 0;
  stream >> argc;
  Q_ASSERT(argc <= 10);

  CuteIPCMessage::Arguments args;
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

  return CuteIPCMessage(type, method, args, returnType);
}


CuteIPCMessage::MessageType CuteIPCMarshaller::demarshallMessageType(QByteArray& message)
{
  QDataStream stream(&message, QIODevice::ReadOnly);
  // Call type
  int buffer;
  stream >> buffer;
  return CuteIPCMessage::MessageType(buffer);
}


CuteIPCMessage CuteIPCMarshaller::demarshallResponse(QByteArray &call, QGenericReturnArgument arg)
{
  QDataStream stream(&call, QIODevice::ReadOnly);

  CuteIPCMessage::MessageType type;
  int buffer;
  stream >> buffer;
  type = CuteIPCMessage::MessageType(buffer);

  QString method;
  stream >> method;

  QString returnType;
  stream >> returnType;

  CuteIPCMessage::Arguments args; //construct message with empty arguments

  int argc;
  stream >> argc;
  if (argc != 0) //load returned value to the arg (not to the message)
  {
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

    bool dataLoaded = QMetaType::load(stream, type, arg.data());
    if (!dataLoaded)
    {
      qWarning() << "Failed to deserialize argument value" << "of type" << typeName;
    }
  }

  return CuteIPCMessage(type, method, args, returnType);
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


void CuteIPCMarshaller::freeArguments(const CuteIPCMessage::Arguments& args)
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
