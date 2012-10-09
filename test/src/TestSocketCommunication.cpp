// Local
#include "TestSocketCommunication.h"
#include "ServiceTestObject.h"
#include "InterfaceTestObject.h"
#include "SignalWaiter.h"

#include <CuteIPCInterface.h>

// Qt
#include <QtTest/QtTest>
#include <QObject>


void TestSocketCommunication::init()
{
  m_service = new ServiceTestObject(0);
  m_interface = new CuteIPCInterface(0);
  QVERIFY(m_service->listen("TestSocket"));
  QVERIFY(m_interface->connectToServer("TestSocket"));
}


void TestSocketCommunication::cleanup()
{
  delete m_interface;
  delete m_service;
}


void TestSocketCommunication::testServerStop()
{
  m_service->close();

  // new connections is not available
  CuteIPCInterface* newInterface = new CuteIPCInterface(this);
  QVERIFY(!newInterface->connectToServer("TestSocket"));

  // new listening
  QVERIFY(m_service->listen("NewSocket"));
  QVERIFY(newInterface->connectToServer("NewSocket"));

  delete newInterface;
}


void TestSocketCommunication::testReconnect()
{
  m_interface->disconnectFromServer();
  delete m_service;
  m_service = new ServiceTestObject(this);
  QVERIFY(m_service->listen("NewSocket"));
  QVERIFY(m_interface->connectToServer("NewSocket"));
//  m_interface->callNoReply("testMethod"); //check that socket is valid (no segfault)
}


void TestSocketCommunication::testDirectCalls()
{
  int intval;

  //test QByteArray transfer
  QByteArray testByteArray(10 * 1024 * 1024, 'H');

  QVERIFY(m_interface->call("testQByteArrayTransfer", Q_RETURN_ARG(int, intval),
                            Q_ARG(QByteArray, testByteArray)) == true);

  QCOMPARE(intval, testByteArray.size());
  QCOMPARE(testByteArray, m_service->getByteArray());


  //test QImage transfer
  QImage testImage(800, 600, QImage::Format_RGB888);
  QVERIFY(m_interface->call("testQImageTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QImage, testImage)) == true);
  QCOMPARE(testImage.size(), m_service->getImage().size());
  QCOMPARE(testImage.pixel(0, 0), m_service->getImage().pixel(0, 0));
  QCOMPARE(testImage.pixel(50, 50), m_service->getImage().pixel(50, 50));

  //test QString transfer
  QString testString("testCallString");
  QVERIFY(m_interface->call("testQStringTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QString, testString)) == true);

  QCOMPARE(intval, testString.size());
  QCOMPARE(testString, m_service->getString());


  //test int transfer without return value
  int testInt = 15;
  QVERIFY(m_interface->call("testIntTransfer", Q_ARG(int, testInt)) == true);

  QCOMPARE(testInt, m_service->getInt());
}


void TestSocketCommunication::testRemoteSignals()
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


void TestSocketCommunication::testLocalSignals()
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

  SignalWaiter waiter;
  waiter.addConnection(m_service, SIGNAL(slotWasCalled(QString)), 3);

  //test transfers
  QByteArray testByteArray(1 * 1024 * 1024, 'B');

  QImage testImage(800, 600, QImage::Format_RGB888);
  QString testString("testLocalSignalsString");
  int testInt = 30;

  firstTestObject->emitQByteArraySignal(testByteArray);
  firstTestObject->emitQImageSignal(testImage);
  secondTestObject->emitQStringIntSignal(testString, testInt);
  QVERIFY(waiter.wait());

  //Compare result values
  QCOMPARE(testByteArray, m_service->getByteArray());
  QCOMPARE(testImage.size(), m_service->getImage().size());
  QCOMPARE(testImage.pixel(0, 0), m_service->getImage().pixel(0, 0));
  QCOMPARE(testImage.pixel(50, 50), m_service->getImage().pixel(50, 50));

  QCOMPARE(testString, m_service->getString());
  QCOMPARE(testInt, m_service->getInt());

  delete firstTestObject;
  delete secondTestObject;
}


void TestSocketCommunication::testRemoteSignalToSignal()
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


void TestSocketCommunication::benchmarkQByteArrayTransfer()
{
  QByteArray testByteArray(QBYTEARRAY_SIZE_FOR_BENCHMARK, 'H');
  int intval;
  qDebug() << "Test QByteArray size:" << testByteArray.size();

  QBENCHMARK_ONCE {
     if (!m_interface->call("testQByteArrayTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QByteArray, testByteArray)))
       QSKIP("Remote call fail. Maybe you need to run TestSocketCommunication tests", SkipAll);
  }
}


void TestSocketCommunication::benchmarkQImageTransfer()
{
  //test QImage transfer
  int intval;
  QImage testImage(QIMAGE_HEIGHT_WIDTH_FOR_BENCHMARK, QIMAGE_HEIGHT_WIDTH_FOR_BENCHMARK, QImage::Format_RGB888);
  qDebug() << "Test QImage size:" << testImage.byteCount();

  QBENCHMARK_ONCE {
     if (!m_interface->call("testQImageTransfer", Q_RETURN_ARG(int, intval), Q_ARG(QImage, testImage)))
       QSKIP("Remote call fail. Maybe you need to run TestSocketCommunication tests", SkipAll);
  }
}


void TestSocketCommunication::testGrayScaleQImageTransfer()
{
  // Gray scale image
  QImage monoImage(":/images/finger.png");
  qDebug() << "Test gray scale QImage size:" << monoImage.byteCount();

  int intval;
  if (!m_interface->call("testQImageGrayScale", Q_RETURN_ARG(int, intval), Q_ARG(QImage, monoImage)))
    QSKIP("Remote call for grey scale fail", SkipAll);
  else
    QVERIFY(intval == monoImage.byteCount());
}


void TestSocketCommunication::testMultipleObjectsConnection()
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

  delete secondTestObject;
}


void TestSocketCommunication::testMultipleClients()
{
  // The same test as testMultipleObjectsConnection(), except:
  // FirstTestObject is managed by m_interface,
  // while secondTestObject is managed by anotherInterface
  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);
  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);
  CuteIPCInterface* anotherInterface = new CuteIPCInterface(this);
  if (!anotherInterface->connectToServer("TestSocket"))
    QSKIP("Can't connect another interface to the server. Skip test", SkipSingle);


  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), firstTestObject, SLOT(interfaceIntSlot(int))));
  QVERIFY(anotherInterface->remoteConnect(SIGNAL(serviceIntSignal(int)), secondTestObject, SLOT(interfaceIntSlot(int))));

  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceQStringSignal(QString)), firstTestObject,
                                     SLOT(interfaceQStringSlot(QString))));
  QVERIFY(anotherInterface->remoteConnect(SIGNAL(serviceQStringIntSignal(QString,int)), secondTestObject,
                                          SLOT(interfaceQStringIntSlot(QString,int))));
  sleep(1000);

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


void TestSocketCommunication::testSignalAfterReturnCall()
{
  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);

  //connected to the first object
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), secondTestObject, SLOT(interfaceIntSlot(int))));
  sleep(1000);

  SignalWaiter waiter;
  waiter.addConnection(secondTestObject, SIGNAL(slotWasCalled(QString)), 1);
  waiter.addConnection(m_service, SIGNAL(serviceIntSignal(int)), 1);

  int testInt = 25;
  QVERIFY(m_interface->call("testCallWithRemoteSignal", Q_ARG(int, testInt)) == true);
  QVERIFY(waiter.wait());

  QCOMPARE(testInt, m_service->getInt());
  delete secondTestObject;
}


void TestSocketCommunication::testRemoteSignalToMultipleSlots()
{
  //one remote signal is connected to multiple slots of the same object
  InterfaceTestObject* testObject = new InterfaceTestObject(this);

  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceIntSlot(int))));

  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceAnotherIntSlot(int))));
  sleep(1000);

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


void TestSocketCommunication::testLocalSignalToMultipleSlots()
{
  // the one local signal is connected to multiple remote slots
  InterfaceTestObject* testObject = new InterfaceTestObject(this);

  QVERIFY(m_interface->remoteSlotConnect(testObject, SIGNAL(interfaceIntSignal(int)), SLOT(serviceIntSlot(int))));
  QVERIFY(m_interface->remoteSlotConnect(testObject, SIGNAL(interfaceIntSignal(int)), SLOT(serviceAnotherIntSlot(int))));

  SignalWaiter waiter;
  waiter.addConnection(m_service, SIGNAL(slotWasCalled(QString)), 1);
  waiter.addConnection(m_service, SIGNAL(anotherSlotWasCalled(QString)), 1);

  int testInt = 30;
  testObject->emitIntSignal(testInt);
  QVERIFY(waiter.wait());

  QVERIFY(m_interface->disconnectSlot(testObject, SIGNAL(interfaceIntSignal(int)), SLOT(serviceAnotherIntSlot(int))));
  QSignalSpy anotherSpyForService(m_service, SIGNAL(anotherSlotWasCalled(QString)));

  waiter.addConnection(m_service, SIGNAL(slotWasCalled(QString)), 1);
  testObject->emitIntSignal(testInt);
  QVERIFY(waiter.wait());

  QCOMPARE(anotherSpyForService.count(), 0);
  delete testObject;
}


void TestSocketCommunication::testOwnersOnTheServerSide()
{
  m_interface->disconnectFromServer();
  CuteIPCService* service = new CuteIPCService(this);
  QVERIFY(service->listen("TestServiceSocket", m_service));
  QVERIFY(m_interface->connectToServer("TestServiceSocket"));

  InterfaceTestObject* testObject = new InterfaceTestObject(this);

  //test remote signals
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), testObject, SLOT(interfaceIntSlot(int))));

  SignalWaiter waiter;
  waiter.addConnection(testObject, SIGNAL(slotWasCalled(QString)), 1);

  int testInt = 25;

  sleep(1000);
  m_interface->callNoReply("emitIntSignal", Q_ARG(int, testInt));
  QVERIFY(waiter.wait());

  //test remote slots
  QVERIFY(m_interface->remoteSlotConnect(testObject, SIGNAL(interfaceQStringSignal(QString)),
                                         SLOT(serviceQStringSlot(QString))));

  waiter.addConnection(m_service, SIGNAL(slotWasCalled(QString)), 1);
  QString testString("testLocalSignalsString");
  testObject->emitQStringSignal(testString);
  QVERIFY(waiter.wait());

  //test direct call
  testInt = 10;
  QVERIFY(m_interface->call("testIntTransfer", Q_ARG(int, testInt)) == true);
  QCOMPARE(testInt, m_service->getInt());

  m_interface->disconnectFromServer();
  delete service;
}


void TestSocketCommunication::connectTime()
{
  m_service = new ServiceTestObject(0);
  QVERIFY(m_service->listen("TestTimeSocket"));

  QTime time;
  time.start();
  m_interface = new CuteIPCInterface(0);
  QVERIFY(m_interface->connectToServer("TestTimeSocket"));
  qDebug() << "time:" << time.elapsed();
}


void TestSocketCommunication::testThread()
{
  QThread* thread = new QThread;
  CuteIPCInterface* newInterface = new CuteIPCInterface;
  QVERIFY(newInterface->connectToServer("TestSocket"));
  newInterface->moveToThread(thread);
  thread->start();

  //test QByteArray transfer, call from another thread
  int intval = 0;
  QByteArray testByteArray(10 * 1024 * 1024, 'B');

  QVERIFY(m_interface->call("testQByteArrayTransfer", Q_RETURN_ARG(int, intval),
                            Q_ARG(QByteArray, testByteArray)) == true);

  QCOMPARE(intval, testByteArray.size());
  QCOMPARE(testByteArray, m_service->getByteArray());

  delete newInterface;
  thread->quit();
  thread->wait();
  delete thread;
}

QString res2;
void TestSocketCommunication::testSimultaneousCalls()
{
  QThread* thread = new QThread;
  m_service->moveToThread(thread);
  thread->start();

  InterfaceTestObject* secondTestObject = new InterfaceTestObject(this);
  QSignalSpy spyForService(m_service, SIGNAL(serviceIntSignal(int)));

  //connected to the first object
  QVERIFY(m_interface->remoteConnect(SIGNAL(serviceIntSignal(int)), this, SLOT(specialSlot(int))));
  sleep(1000);

  int testInt = 25;
  int res1;
  QVERIFY(m_interface->call("testCallWithRemoteSignal", Q_RETURN_ARG(int, res1), Q_ARG(int, testInt)) == true);

  QCOMPARE(spyForService.count(), 1);
  QCOMPARE(m_service->getInt(), 25);
  QCOMPARE(m_service->getString(), QString("test"));

  QCOMPARE(res1, 40);
  QCOMPARE(res2, QString("testtest"));
  delete secondTestObject;

  thread->quit();
  thread->wait();
  delete thread;
}


void TestSocketCommunication::specialSlot(int)
{
  qDebug() << "invoke special slot...";
  QString testString("test");
  QVERIFY(m_interface->call("testQStringTransfer2", Q_RETURN_ARG(QString, res2), Q_ARG(QString, testString)) == true);
  qDebug() << "...done";
  emit specialSlotFinished();
}


void TestSocketCommunication::sleep(int msecs)
{
  QEventLoop loop;
  QTimer timer;
  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  timer.start(msecs);
  loop.exec();
}

QTEST_MAIN(TestSocketCommunication)
