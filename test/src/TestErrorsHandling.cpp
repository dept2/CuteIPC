// Local
#include "TestErrorsHandling.h"
#include "ServiceTestObject.h"
#include "InterfaceTestObject.h"

// CuteIPC
#include <CuteIPCService.h>
#include <CuteIPCInterface.h>

// Qt
#include <QtTest/QtTest>
#include <QObject>


void TestErrorsHandling::init()
{
  m_service = new ServiceTestObject(this);
  m_interface = new CuteIPCInterface(this);
}


void TestErrorsHandling::testWrongConnection()
{
  QVERIFY(m_service->listen("TestSocket"));
  QVERIFY(!m_interface->connectToServer("UnexistedSocket"));
  QVERIFY(m_interface->connectToServer("TestSocket"));

  delete m_service;
  delete m_interface;
}


void TestErrorsHandling::testInvokeErrors()
{
  QVERIFY(m_service->listen("TestSocket"));
  QVERIFY(m_interface->connectToServer("TestSocket"));


  int testInt = 5;
  //unexisted method
  QByteArray testByteArray(10 * 1024 * 1024, 'M');
  QVERIFY(!m_interface->call("unexistedMethod",
                             Q_RETURN_ARG(int, testInt),
                             Q_ARG(QByteArray, testByteArray)));
  QCOMPARE(m_interface->lastError(), QString("Unsuccessful invoke"));

  //existed method, but wrong parameters
  QVERIFY(!m_interface->call("testQByteArrayTransfer",
                             Q_RETURN_ARG(int, testInt),
                             Q_ARG(int, testInt)));
  QCOMPARE(m_interface->lastError(), QString("Unsuccessful invoke"));

  //existed method, but wrong return type
  QVERIFY(!m_interface->call("testQByteArrayTransfer",
                             Q_RETURN_ARG(QByteArray, testByteArray),
                             Q_ARG(QByteArray, testByteArray)));
  QCOMPARE(m_interface->lastError(), QString("Unsuccessful invoke"));

  delete m_service;
  delete m_interface;
}


void TestErrorsHandling::testRemoteSignalsErrors()
{
  QVERIFY(m_service->listen("TestSocket"));
  QVERIFY(m_interface->connectToServer("TestSocket"));

  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);

  //incompatible signatures
  QVERIFY(!m_interface->remoteConnect(SIGNAL(serviceQByteArraySignal(QByteArray)),
                                      firstTestObject,
                                      SLOT(interfaceQByteArraySlot(int))));
  QCOMPARE(m_interface->lastError(),
           QString("Incompatible signatures: serviceQByteArraySignal(QByteArray),interfaceQByteArraySlot(int)"));

  //unexisted slot
  QVERIFY(!m_interface->remoteConnect(SIGNAL(serviceQByteArraySignal(QByteArray)),
                                      firstTestObject,
                                      SLOT(unexistedSlot(QByteArray))));
  QCOMPARE(m_interface->lastError(),
           QString("Slot doesn't exist:unexistedSlot(QByteArray)"));

  //unexisted remote signal
  QVERIFY(!m_interface->remoteConnect(SIGNAL(unexistedSignal(QByteArray)),
                                      firstTestObject,
                                      SLOT(interfaceQByteArraySlot(QByteArray))));
  QCOMPARE(m_interface->lastError(), QString("Signal doesn't exist:unexistedSignal(QByteArray)"));

  delete firstTestObject;
  delete m_interface;
  delete m_service;
}


void TestErrorsHandling::testRemoteSlotErrors()
{
  QVERIFY(m_service->listen("TestSocket"));
  QVERIFY(m_interface->connectToServer("TestSocket"));

  InterfaceTestObject* firstTestObject = new InterfaceTestObject(this);

  //incompatible signatures
  QVERIFY(!m_interface->remoteSlotConnect(firstTestObject,
                                          SIGNAL(interfaceQByteArraySignal(QByteArray)),
                                          SLOT(serviceQByteArraySlot(int))));
  QCOMPARE(m_interface->lastError(),
           QString("Incompatible signatures: interfaceQByteArraySignal(QByteArray),serviceQByteArraySlot(int)"));


  //unexisted signal
  QVERIFY(!m_interface->remoteSlotConnect(firstTestObject,
                                          SIGNAL(interfaceQImageSignal(QByteArray)),
                                          SLOT(serviceQByteArraySlot(QByteArray))));
  QCOMPARE(m_interface->lastError(),
           QString("Signal doesn't exist:interfaceQImageSignal(QByteArray)"));


  //unexisted remote slot
  QVERIFY(!m_interface->remoteSlotConnect(firstTestObject,
                                          SIGNAL(interfaceQImageSignal(QImage)),
                                          SLOT(unexistedSlot(QImage))));
  QCOMPARE(m_interface->lastError(),
           QString("Remote slot doesn't exist:unexistedSlot(QImage)"));
}


void TestErrorsHandling::sleep(int msecs)
{
  QEventLoop loop;
  QTimer timer;
  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  timer.start(msecs);
  loop.exec();
}

QTEST_MAIN(TestErrorsHandling)
