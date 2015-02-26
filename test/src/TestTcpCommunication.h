#ifndef TESTTCPCOMMUNICATION_H
#define TESTTCPCOMMUNICATION_H

// Qt
#include <QtTest/QtTest>

// Local
class ServiceTestObject;
class CuteIPCInterface;

class TestTcpCommunication : public QObject
{
  Q_OBJECT

  static const int QBYTEARRAY_SIZE_FOR_BENCHMARK = 200 * 1024 * 1024;
  static const int QIMAGE_HEIGHT_WIDTH_FOR_BENCHMARK = 8000;

  private slots:
    // init, cleanup
    void init();
    void cleanup();

    // basic communication
    void testReconnect();

    // basic data transfer tests
    void testDirectCalls();
    void testRemoteSignals();
    void testRemoteSignalsWithSyncCall();
    void testLocalSignals();
    void testRemoteSignalToSignal();

    // benchmark tests
    void benchmarkQByteArrayTransfer();
    void benchmarkQImageTransfer();

    // image tests
    void testImageDPI();
    void testGrayScaleQImageTransfer();

    // 'difficult' connection cases
    void testMultipleObjectsConnection();
    void testMultipleClients();
    void testRemoteSignalToMultipleSlots();

    // thread tests
    void testThread();

    // finish
    void tcpFinish();

  public slots:
    void specialSlot(int);

  signals:
    void specialSlotFinished();

  private:
    CuteIPCInterface* m_interface;

    void sleep(int);
};

#endif //TESTLOCALTCPCOMMUNICATION_H
