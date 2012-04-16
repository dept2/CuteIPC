#ifndef TESTOBJECTSLOT_H
#define TESTOBJECTSLOT_H

#include <QObject>

class TestObjectSlot : public QObject
{
  Q_OBJECT

  public:
    TestObjectSlot(QObject* parent = 0);
    ~TestObjectSlot() {}

  signals:
    void testSignal(const QString& msg);

  public slots:
    void debugSlot(const QString& msg, int i = 5);
    void emitSignal(const QString& msg);
};

#endif // TESTOBJECTSLOT_H
