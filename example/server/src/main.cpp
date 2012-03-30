// Qt
#include <QCoreApplication>

// Local
#include "TestObject.h"


/*void debugCallOutput(QByteArray call)
{
  qDebug() << "---" << Q_FUNC_INFO;

  QDataStream stream(&call, QIODevice::ReadOnly);

  QString method;
  stream >> method;
  qDebug() << "Method name:" << method << "(call size:" << call.size() << "bytes)";

  int argc = 0;
  stream >> argc;
  qDebug() << "Args count:" << argc;

  for (int i = 0; i < argc; ++i)
  {
    // Load type
    QString typeName;
    stream >> typeName;
    qDebug() << "  arg" << i << ":" << typeName;

    // Check type
    int type = QMetaType::type(typeName.toLatin1());
    if (type == 0)
    {
      qWarning() << "Unsupported type of argument" << i << ":" << typeName;
      break;
    }

    // Read argument data from stream
    void* data = QMetaType::construct(type);
    bool ok = QMetaType::load(stream, type, data);
    if (!ok)
    {
      qWarning() << "Failed to deserialize argument" << i << "of type" << typeName;
      break;
    }
  }

  qDebug("---");
}*/


int main(int argc, char* argv[])
{
  QCoreApplication a(argc, argv);

  TestObject obj;
  obj.listen("TestObject");

  return a.exec();
}
