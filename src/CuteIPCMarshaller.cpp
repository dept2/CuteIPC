// Local
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QDataStream>
#include <QPair>
#include <QMetaType>
#include <QDebug>


QByteArray CuteIPCMarshaller::marshallCall(const QString& method, QGenericArgument val0, QGenericArgument val1,
    QGenericArgument val2, QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
    QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  QByteArray result;
  QDataStream stream(&result, QIODevice::WriteOnly);

  // Method name
  stream << method;

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

  foreach (const QGenericArgument& arg, args)
  {
    // Detect and check type
    int type = QMetaType::type(arg.name());
    if (type == 0)
    {
      qWarning() << "Type" << arg.name() << "have not been registered in Qt metaobject system";
      return QByteArray();
    }

    stream << QString::fromLatin1(arg.name());
    bool ok = QMetaType::save(stream, type, arg.data());
    if (!ok)
    {
      qWarning() << "Failed to serialize" << arg.name() << "to data stream. Call qRegisterMetaTypeStreamOperators to"
                    " register stream operators for this metatype";
      return QByteArray();
    }
  }

  return result;
}


CuteIPCMarshaller::Call CuteIPCMarshaller::demarshallCall(QByteArray call)
{
  QDataStream stream(&call, QIODevice::ReadOnly);

  // Method
  QString method;
  stream >> method;

  // Arguments count
  int argc = 0;
  stream >> argc;
  Q_ASSERT(argc <= 10);

  Arguments args;

  for (int i = 0; i < argc; ++i)
  {
    // Load type
    QString typeName;
    stream >> typeName;

    // Check type
    int type = QMetaType::type(typeName.toLatin1());
    if (type == 0)
    {
      qWarning() << "Unsupported type of argument" << i << ":" << typeName;
      break; // TODO quit (don't forget to clear the already allocated memory
    }

    // Read argument data from stream
    void* data = QMetaType::construct(type);
    bool ok = QMetaType::load(stream, type, data);
    if (!ok)
    {
      qWarning() << "Failed to deserialize argument" << i << "of type" << typeName;
      break; // TODO quit (don't forget to clear the already allocated memory
    }

    args.append(QGenericArgument(qstrdup(typeName.toLatin1()), data));
  }

  // Fill empty args
  while (args.size() < 10)
    args.append(QGenericArgument());

  return qMakePair(method, args);
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
