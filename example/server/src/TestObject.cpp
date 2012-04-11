// Local
#include "TestObject.h"

// Qt
#include <QDebug>


TestObject::TestObject(QObject* parent)
: CuteIPCService(parent)
{}


int TestObject::bar(const QByteArray& ba)
{
  qDebug() << Q_FUNC_INFO << ba.size();
  emit testSignal(QString("Byte array size: %1").arg(ba.size()));
  return ba.size();
}


void TestObject::foo(const QString& msg)
{
  qDebug() << Q_FUNC_INFO << msg;
}
