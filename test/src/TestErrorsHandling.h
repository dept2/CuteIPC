#ifndef TESTERRORSHANDLING_H
#define TESTERRORSHANDLING_H

// Qt
#include <QtTest/QtTest>

// Local
class ServiceTestObject;
class CuteIPCInterface;

class TestErrorsHandling : public QObject
{
  Q_OBJECT

  private slots:

    // basic communications tests
    void init();

    void testWrongConnection();
    void testInvokeErrors();
    void testRemoteSignalsErrors();
    void testRemoteSlotErrors();

  private:
    ServiceTestObject* m_service;
    CuteIPCInterface* m_interface;

    void sleep(int);
};

#endif //TESTERRORSHANDLING_H
