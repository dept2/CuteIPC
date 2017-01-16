#ifndef TESTMESSAGEMARSHALLING_H
#define TESTMESSAGEMARSHALLING_H

#include <QtTest/QtTest>

class TestMessageMarshalling : public QObject
{
  Q_OBJECT

  private slots:
    void init();

    void constructMessageWithArgs();
    void constructMessageWithoutArgs();

    void marshallMessageParameters();
    void marshallIntergers();
    void marshallLiterals();
    void marshallQImageRGB888();
    void marshallQImageIndexed8();
    void marshallQListOfQImage();

  private:
    QImage createRGB888Image() const;
    QImage createIndexed8Image() const;
};

#endif // TESTMESSAGEMARSHALLING_H
