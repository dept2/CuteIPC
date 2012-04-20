#ifndef TESTSOCKETCOMMUNICATION_H
#define TESTSOCKETCOMMUNICATION_H

// Qt
#include <QtTest/QtTest>

// Local
class ServiceTestObject;
class CuteIPCInterface;

class TestSocketCommunication: public QObject
{
  static const int QBYTEARRAY_SIZE_FOR_BENCHMARK = 200 * 1024 * 1024;
  static const int QIMAGE_HEIGHT_WIDTH_FOR_BENCHMARK = 8000;

  Q_OBJECT

  private slots:

    // basic communications tests
    void init();
    void cleanup();
    void testServerStop();
    void testClientDisconnect();

    // basic data transfer tests
    void testDirectCalls();
    void testRemoteSignals();
    void testLocalSignals();

    // benchmark tests
    void benchmarkQByteArrayTransfer();
    void benchmarkQImageTransfer();

    //TODO: need to write these tests:
    void testMultipleObjectsConnection();
    void testMultipleClients();

  private:
    ServiceTestObject* m_service;
    CuteIPCInterface* m_interface;

    void sleep(int);
};

#endif //TESTSOCKETCOMMUNICATION_H
