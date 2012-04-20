#ifndef TESTSOCKETCOMMUNICATION_H
#define TESTSOCKETCOMMUNICATION_H

// Qt
#include <QtTest/QtTest>

// Local
class ServiceTestObject;
class CuteIPCInterface;

class TestSocketCommunication: public QObject
{
  Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testConnection();
    void testDirectCalls();
    void testRemoteSignals();
    void testLocalSignals();

    //TODO: need to write these tests:
    void testMultipleClientConnections();
    void testNestedSignals();
    void testRemoteSignalsDisconnect();
    void testLocalSignalsDisconnect();

  private:
    ServiceTestObject* m_service;
    CuteIPCInterface* m_interface;
};

#endif //TESTSOCKETCOMMUNICATION_H
