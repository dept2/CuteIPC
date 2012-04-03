// Local
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QDataStream>
#include <QPair>
#include <QMetaType>
#include <QDebug>


QByteArray CuteIPCMarshaller::marshallCall(const QString& method,
    QGenericArgument val0, QGenericArgument val1,
    QGenericArgument val2, QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
    QGenericArgument val7, QGenericArgument val8, QGenericArgument val9, QString retType)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);

  // Method name
  stream << method;

  // "isVoid" boolean value and it's type
  if (retType.isEmpty())
  {
    stream << true;
  }
  else
  {
    stream << false;
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

QByteArray CuteIPCMarshaller::marshallReturnedValue(QGenericArgument returnedValue)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);

  bool successfullyMarshalled = marshallArgumentToStream(returnedValue, stream);
  if (!successfullyMarshalled)
    return QByteArray();
  return result;
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
      qWarning() << "Failed to serialize" << returnedValue.name() << "to data stream. Call qRegisterMetaTypeStreamOperators to"
                    " register stream operators for this metatype";
      return false;
    }

  return true;
}

CuteIPCMarshaller::Call CuteIPCMarshaller::demarshallCall(QByteArray call)
{
  QDataStream stream(&call, QIODevice::ReadOnly);

  // Method
  QString method;
  stream >> method;

  bool isVoid;
  QString retType;
  stream >> isVoid;
  if (!isVoid)
  {
    stream >> retType;
  }

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

  return Call(method, args, retType);
//  return qMakePair(method, args);
}

QGenericArgument CuteIPCMarshaller::demarshallReturnedValue(QByteArray value)
{
  QDataStream stream(&value, QIODevice::ReadOnly);

  bool ok;
  QGenericArgument returnedValue = demarshallArgumentFromStream(ok, stream);
  if (!ok)
    qDebug() << "Failed to demarshall returned value";
  return returnedValue;
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


CuteIPCMarshaller::Call::Call(QString method, Arguments arguments, QString retType)
{
  this->first = method;
  this->second = arguments;
  this->retType = retType;
}
