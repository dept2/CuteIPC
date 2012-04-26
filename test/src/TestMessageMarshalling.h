#ifndef TESTMESSAGEMARSHALLING_H
#define TESTMESSAGEMARSHALLING_H

#include <QtTest/QtTest>

class TestMessageMarshalling : public QObject
{
  Q_OBJECT

  private slots:
    void constructMessageWithArgs();
    void constructMessageWithoutArgs();

    void marshallMessageParameters();
    void marshallIntergers();
    void marshallLiterals();
    void marshallQImages();
};

#endif // TESTMESSAGEMARSHALLING_H
