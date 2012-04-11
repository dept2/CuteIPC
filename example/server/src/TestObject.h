#ifndef TESTOBJECT_H
#define TESTOBJECT_H

// Local
#include "CuteIPCService.h"


class TestObject : public CuteIPCService
{
  Q_OBJECT

  public:
    TestObject(QObject* parent = 0);

    Q_INVOKABLE int bar(const QByteArray&);

  public slots:
    void foo(const QString& msg);

  signals:
    void testSignal(const QString& message);
};

#endif // TESTOBJECT_H
