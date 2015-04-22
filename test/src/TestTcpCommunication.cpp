// Local
#include "TestTcpCommunication.h"
#include "ServiceTestObject.h"
#include "InterfaceTestObject.h"
#include "SignalWaiter.h"

#include <CuteIPCInterface.h>

// Qt
#include <QtTest/QtTest>
#include <QObject>


namespace
{
  const int primaryPort = 5555;
  const char* const host = "127.0.0.1";
}


void TestTcpCommunication::init()
{
  m_interface = new CuteIPCInterface(0);
  QTime time;
  time.start();
  QVERIFY2(m_interface->connectToServer(QHostAddress(host), primaryPort), "Connection failed. Check if TestTcpServer started");
  qDebug() << "Connect time:" << time.elapsed();
}


void TestTcpCommunication::cleanup()
{
  delete m_interface;
}


void TestTcpCommunication::testReconnect()
{
  m_interface->disconnectFromServer();
  QVERIFY(m_interface->connectToServer(QHostAddress(host), primaryPort));
}


void TestTcpCommunication::testDirectCalls()
{
  int intval;

  //test QByteArray transfer
  QByteArray testByteArray(10 * 1024 * 1024, 'H');

  QVERIFY(m_interface->call("testQByteArrayTransfer", Q_RETURN_ARG(int, intval),
                            Q_ARG(QByteArray, testByteArray)) == true);

  QCOMPARE(intval, testByteArray.size());

  //test QImage transfer
  QImage testImage(800, 600, QImage::Format_RGB888);
  QVERIFY(m_interface->call("testQImageTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QImage, testImage)) == true);

  // Test QImage return
  QImage returnedImage;
  QVERIFY(m_interface->call("getImage", Q_RETURN_ARG(QImage, returnedImage)));
  qDebug() << "ret" << returnedImage.format() << returnedImage.size();
  QCOMPARE(testImage, returnedImage);

  //test QString transfer
  QString testString("testCallString");
  QVERIFY(m_interface->call("testQStringTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QString, testString)) == true);

  QCOMPARE(intval, testString.size());

  //test int transfer without return value
  int testInt = 15;
  QVERIFY(m_interface->call("testIntTransfer", Q_ARG(int, testInt)) == true);
}


void TestTcpCommunication::testRemoteSignals()
{
  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);
  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);

  //connected to the first object
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQByteArraySignal(QByteArray)), firstTestObject,
                                     SLOT(interfaceQByteArraySlot(QByteArray))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQImageSignal(QImage)), firstTestObject,
                                     SLOT(interfaceQImageSlot(QImage))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringIntSignal(QString,int)), secondTestObject,
                                     SLOT(interfaceQStringIntSlot(QString,int))));

  //connected, but not emited
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), secondTestObject, SLOT(interfaceIntSlot(int))));

  SignalWaiter waiter;
  waiter.addConnection(firstTestObject, SIGNAL(slotWasCalled(QString)), 2);
  waiter.addConnection(secondTestObject, SIGNAL(slotWasCalled(QString)), 1);

  //test transfers
  QByteArray testByteArray(1 * 1024 * 1024, 'A');

  QImage testImage(800, 600, QImage::Format_RGB888);
  QString testString("testRemoteSignalsString");
  int testInt = 25;

  m_interface->callNoReply("emitQByteArraySignal", Q_ARG(QByteArray, testByteArray));
  m_interface->callNoReply("emitQImageSignal", Q_ARG(QImage, testImage));
  m_interface->callNoReply("emitQStringIntSignal", Q_ARG(QString, testString), Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  //Compare result values
  QCOMPARE(testByteArray, firstTestObject->getByteArray());
  QCOMPARE(testImage.size(), firstTestObject->getImage().size());
  QCOMPARE(testImage.pixel(0, 0), firstTestObject->getImage().pixel(0, 0));
  QCOMPARE(testImage.pixel(50, 50), firstTestObject->getImage().pixel(50, 50));

  QCOMPARE(testString, secondTestObject->getString());
  QCOMPARE(testInt, secondTestObject->getInt());

  delete firstTestObject;
  delete secondTestObject;
}


void TestTcpCommunication::testRemoteSignalsWithSyncCall()
{
  InterfaceTestObject* testObject = new InterfaceTestObject(this);
  SignalWaiter waiter;
  waiter.addConnection(testObject, SIGNAL(slotWasCalled(QString)), 1);

  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringSignal(QString)), testObject, SLOT(interfaceQStringSlot(QString))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringIntSignal(QString,int)), testObject, SLOT(interfaceQStringIntSlot(QString,int))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQByteArraySignal(QByteArray)), testObject, SLOT(interfaceQByteArraySlot(QByteArray))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQImageSignal(QImage)), testObject, SLOT(interfaceQImageSlot(QImage))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceIntSlot(int))));

  // all connections must be established before the following call
  QVERIFY(m_interface->call("emitIntSignal", Q_ARG(int, 5)));

  QVERIFY(waiter.wait());
}


void TestTcpCommunication::testLocalSignals()
{
  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);
  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);

  QVERIFY(m_interface->remoteSlotConnect(firstTestObject, SIGNAL(interfaceQByteArraySignal(QByteArray)),
                                         SLOT(serviceQByteArraySlot(QByteArray))));
  QVERIFY(m_interface->remoteSlotConnect(firstTestObject, SIGNAL(interfaceQImageSignal(QImage)),
                                         SLOT(serviceQImageSlot(QImage))));

  QVERIFY(m_interface->remoteSlotConnect(secondTestObject, SIGNAL(interfaceQStringIntSignal(QString,int)),
                                         SLOT(serviceQStringIntSlot(QString,int))));

  QVERIFY(m_interface->remoteSlotConnect(secondTestObject, SIGNAL(interfaceQStringSignal(QString)),
                                         SLOT(serviceQStringSlot(QString))));

  QVERIFY(m_interface->remoteConnect(SIGNAL(slotWasCalled(QString)), firstTestObject, SIGNAL(slotWasCalled(QString))));
  SignalWaiter waiter;
  waiter.addConnection(firstTestObject, SIGNAL(slotWasCalled(QString)), 3);

  //test transfers
  QByteArray testByteArray(1 * 1024 * 1024, 'B');
  QImage testImage(800, 600, QImage::Format_RGB888);
  QString testString("testLocalSignalsString");
  int testInt = 30;

  firstTestObject->emitQByteArraySignal(testByteArray);
  firstTestObject->emitQImageSignal(testImage);
  secondTestObject->emitQStringIntSignal(testString, testInt);
  QVERIFY(waiter.wait());

  delete firstTestObject;
  delete secondTestObject;
}


void TestTcpCommunication::testRemoteSignalToSignal()
{
  InterfaceTestObject* testObject = new InterfaceTestObject(this);

  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SIGNAL(interfaceIntSignal(int))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceIntSlot(int))));

  SignalWaiter waiter;
  waiter.addConnection(testObject, SIGNAL(slotWasCalled(QString)), 1);
  waiter.addConnection(testObject, SIGNAL(interfaceIntSignal(int)), 1);

  int testInt = 25;
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  QVERIFY(m_interface->disconnectSignal(SIGNAL(serviceIntSignal(int)), testObject, SIGNAL(interfaceIntSignal(int))));

  waiter.addConnection(testObject, SIGNAL(slotWasCalled(QString)), 1);
  QSignalSpy spyForSignal(testObject, SIGNAL(interfaceIntSignal(int)));

  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  // check disconnect
  QCOMPARE(spyForSignal.count(), 0);

  delete testObject;
}


void TestTcpCommunication::benchmarkQByteArrayTransfer()
{
  QByteArray testByteArray(QBYTEARRAY_SIZE_FOR_BENCHMARK, 'H');
  int intval;
  qDebug() << "Test QByteArray size:" << testByteArray.size();

  QBENCHMARK
  {
    QVERIFY(m_interface->call("testQByteArrayTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QByteArray, testByteArray)));
  }
}


void TestTcpCommunication::benchmarkQImageTransfer()
{
  //test QImage transfer
  int intval;
  QImage testImage(QIMAGE_HEIGHT_WIDTH_FOR_BENCHMARK, QIMAGE_HEIGHT_WIDTH_FOR_BENCHMARK, QImage::Format_RGB888);
  qDebug() << "Test QImage size:" << testImage.byteCount();

  QBENCHMARK
  {
    QVERIFY(m_interface->call("testQImageTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QImage, testImage)));
  }
}


void TestTcpCommunication::testImageDPI()
{
  QImage testImage(640, 480, QImage::Format_ARGB32);
  testImage.fill(Qt::white);
  testImage.setDotsPerMeterX(300 / 2.54 * 100);
  testImage.setDotsPerMeterY(600 / 2.54 * 100);

  QImage returnImage;
  QVERIFY(m_interface->call("testQImageReturn", Q_RETURN_ARG(QImage, returnImage), Q_ARG(QImage, testImage)));
  QCOMPARE(testImage.dotsPerMeterX(), returnImage.dotsPerMeterX());
  QCOMPARE(testImage.dotsPerMeterY(), returnImage.dotsPerMeterY());
}


void TestTcpCommunication::testGrayScaleQImageTransfer()
{
  // Gray scale image
  QImage monoImage(":/images/finger.png");
  qDebug() << "Test gray scale QImage size:" << monoImage.byteCount();

  // Send grayscale image
  int intval;
  QVERIFY(m_interface->call("testQImageGrayScale", Q_RETURN_ARG(int, intval), Q_ARG(QImage, monoImage)));
  QVERIFY(intval == monoImage.byteCount());
}


void TestTcpCommunication::testMultipleObjectsConnection()
{
  //two objects are managed by one CuteIPCClient
  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);
  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);

  //one signal is connected to both clients
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), firstTestObject, SLOT(interfaceIntSlot(int))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), secondTestObject, SLOT(interfaceIntSlot(int))));

  //every client has another (own) connection
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringSignal(QString)), firstTestObject,
                                     SLOT(interfaceQStringSlot(QString))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringIntSignal(QString,int)), secondTestObject,
                                     SLOT(interfaceQStringIntSlot(QString,int))));

  SignalWaiter waiter;
  waiter.addConnection(firstTestObject, SIGNAL(slotWasCalled(QString)), 2);
  waiter.addConnection(secondTestObject, SIGNAL(slotWasCalled(QString)), 2);

  QString testString("testMultipleClientString");
  int testInt = 10;

  m_interface->callNoReply("emitQStringSignal", Q_ARG(QString, testString));
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  m_interface->callNoReply("emitQStringIntSignal", Q_ARG(QString, testString), Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  // delete the one object. The second object must still get 2 signals
  delete firstTestObject;

  waiter.addConnection(secondTestObject, SIGNAL(slotWasCalled(QString)), 2);
  m_interface->callNoReply("emitQStringSignal", Q_ARG(QString, testString));
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  m_interface->callNoReply("emitQStringIntSignal", Q_ARG(QString, testString), Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  secondTestObject->deleteLater();
}


void TestTcpCommunication::testMultipleClients()
{
  // The same test as testMultipleObjectsConnection(), except:
  // FirstTestObject is managed by m_interface,
  // while secondTestObject is managed by anotherInterface
  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);
  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);
  CuteIPCInterface* anotherInterface = new CuteIPCInterface(this);
  QVERIFY(anotherInterface->connectToServer(QHostAddress(host), primaryPort));


  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), firstTestObject, SLOT(interfaceIntSlot(int))));
  QVERIFY(anotherInterface->remoteConnect(SIGNAL(serviceIntSignal(int)), secondTestObject, SLOT(interfaceIntSlot(int))));

  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringSignal(QString)), firstTestObject,
                                     SLOT(interfaceQStringSlot(QString))));
  QVERIFY(anotherInterface->remoteConnect(SIGNAL(serviceQStringIntSignal(QString,int)), secondTestObject,
                                          SLOT(interfaceQStringIntSlot(QString,int))));

  SignalWaiter waiter;
  waiter.addConnection(firstTestObject, SIGNAL(slotWasCalled(QString)), 2);
  waiter.addConnection(secondTestObject, SIGNAL(slotWasCalled(QString)), 2);

  QString testString("testMultipleClientsString");
  int testInt = 10;

  m_interface->callNoReply("emitQStringSignal", Q_ARG(QString, testString));
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  m_interface->callNoReply("emitQStringIntSignal", Q_ARG(QString, testString), Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  // delete the one object. The second object must still get 2 signals
  delete firstTestObject;

  waiter.addConnection(secondTestObject, SIGNAL(slotWasCalled(QString)), 2);
  m_interface->callNoReply("emitQStringSignal", Q_ARG(QString, testString));
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  m_interface->callNoReply("emitQStringIntSignal", Q_ARG(QString, testString), Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  delete secondTestObject;
  delete anotherInterface;
}


void TestTcpCommunication::testRemoteSignalToMultipleSlots()
{
  //one remote signal is connected to multiple slots of the same object
  InterfaceTestObject* testObject = new InterfaceTestObject(this);

  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceIntSlot(int))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceAnotherIntSlot(int))));

  SignalWaiter waiter;
  waiter.addConnection(testObject, SIGNAL(slotWasCalled(QString)), 1);
  waiter.addConnection(testObject, SIGNAL(anotherSlotWasCalled(QString)), 1);

  int testInt = 10;
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  QVERIFY(m_interface->disconnectSignal(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceAnotherIntSlot(int))));

  QSignalSpy anotherSpyForTestObject(testObject, SIGNAL(anotherSlotWasCalled(QString)));
  waiter.addConnection(testObject, SIGNAL(slotWasCalled(QString)), 1);
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  // disconnect test
  QCOMPARE(anotherSpyForTestObject.count(), 0);
  delete testObject;
}


void TestTcpCommunication::testThread()
{
  QThread* thread = new QThread;
  CuteIPCInterface* newInterface = new CuteIPCInterface;
  QVERIFY(newInterface->connectToServer(QHostAddress(host), primaryPort));
  newInterface->moveToThread(thread);
  thread->start();

  //test QByteArray transfer, call from another thread
  int intval = 0;
  QByteArray testByteArray(10 * 1024 * 1024, 'B');

  QVERIFY(m_interface->call("testQByteArrayTransfer", Q_RETURN_ARG(int, intval),
                            Q_ARG(QByteArray, testByteArray)) == true);

  QCOMPARE(intval, testByteArray.size());

  delete newInterface;
  thread->quit();
  thread->wait();
  delete thread;
}


void TestTcpCommunication::testBothLocalAndRemote()
{
  if (QString(host) != "127.0.0.1")
    QSKIP("Can be executed only with localhost server");

  CuteIPCInterface* localInterface = new CuteIPCInterface(this);
  QVERIFY(localInterface->connectToServer("LocalSocket"));

  // direct calls
  int intval;
  QString testString("testCallString");
  QVERIFY(m_interface->call("testQStringTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QString, testString)) == true);
  QCOMPARE(intval, testString.size());
  QVERIFY(localInterface->call("testQStringTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QString, testString)) == true);
  QCOMPARE(intval, testString.size());

  // signals
  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);
  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);
  SignalWaiter waiter;
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringIntSignal(QString,int)), firstTestObject,
                                     SLOT(interfaceQStringIntSlot(QString,int))));
  QVERIFY(localInterface->remoteConnect(SIGNAL(serviceQStringIntSignal(QString,int)), secondTestObject,
                                     SLOT(interfaceQStringIntSlot(QString,int))));
  waiter.addConnection(firstTestObject, SIGNAL(slotWasCalled(QString)), 1);
  waiter.addConnection(secondTestObject, SIGNAL(slotWasCalled(QString)), 1);
  int testInt = 25;
  testString = "testRemoteSignals";
  m_interface->callNoReply("emitQStringIntSignal", Q_ARG(QString, testString), Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  // remote slots
  QVERIFY(m_interface->remoteSlotConnect(firstTestObject, SIGNAL(interfaceQStringIntSignal(QString,int)),
                                         SLOT(serviceQStringIntSlot(QString,int))));
  QVERIFY(localInterface->remoteSlotConnect(secondTestObject, SIGNAL(interfaceQStringIntSignal(QString,int)),
                                         SLOT(serviceQStringIntSlot(QString,int))));
  QVERIFY(m_interface->remoteConnect(SIGNAL(slotWasCalled(QString)), firstTestObject, SIGNAL(slotWasCalled(QString))));
  SignalWaiter waiter2;
  waiter2.addConnection(firstTestObject, SIGNAL(slotWasCalled(QString)), 2);
  testString = "testLocalSignals";
  testInt = 30;

  firstTestObject->emitQStringIntSignal(testString, testInt);
  secondTestObject->emitQStringIntSignal(testString, testInt);
  QVERIFY(waiter2.wait());
}


void TestTcpCommunication::tcpFinish()
{
  m_interface->callNoReply("tcpFinish");
}


QString res2;
void TestTcpCommunication::specialSlot(int)
{
  qDebug() << "invoke special slot...";
  QString testString("test");
  QVERIFY(m_interface->call("testQStringTransfer2", Q_RETURN_ARG(QString, res2), Q_ARG(QString, testString)) == true);
  qDebug() << "...done";
  emit specialSlotFinished();
}


void TestTcpCommunication::sleep(int msecs)
{
  QEventLoop loop;
  QTimer timer;
  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  timer.start(msecs);
  loop.exec();
}

QTEST_MAIN(TestTcpCommunication)
