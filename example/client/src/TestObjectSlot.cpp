// Local
#include "TestObjectSlot.h"

// Qt
#include <QObject>
#include <QDebug>

TestObjectSlot::TestObjectSlot(QObject* parent)
    : QObject(parent)
{}

void TestObjectSlot::debugSlot(const QString& msg, int i)
{
  qDebug() << Q_FUNC_INFO << msg << i;
  qDebug() << "emit testSignal...";
  emit testSignal("testSignal from debug slot");
}


void TestObjectSlot::emitSignal(const QString& msg)
{
  emit testSignal(msg);
}
