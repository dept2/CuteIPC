// Local
#include "TestObject.h"

// Qt
#include <QDebug>
#include <QtGui/QImage>


TestObject::TestObject(QObject* parent)
: CuteIPCService(parent)
{}


int TestObject::bar(const QByteArray& ba)
{
  qDebug() << Q_FUNC_INFO << ba.size();
  emit testSignal(QString("Byte array size: %1").arg(ba.size()));
  emit testSignal2("Another string", 5);
  return ba.size();
}


void TestObject::imagetest(const QImage& image)
{
  image.save("saved","PNG",100);
}


void TestObject::foo(const QString& msg)
{
  qDebug() << Q_FUNC_INFO << msg;
}
