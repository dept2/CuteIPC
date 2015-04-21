// Local
#include "TestTcpServer.h"
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
  const int secondaryPort = 5556;
}


void TestTcpServer::init()
{
  m_service = new ServiceTestObject(0);
  QHostAddress address = QHostAddress::Any;
  qDebug() << "Trying to listen at" << address.toString() << primaryPort;
  QVERIFY(m_service->listenTcp(address, primaryPort));
}


void TestTcpServer::cleanup()
{
  m_service->close();
  delete m_service;
}


void TestTcpServer::generalTest()
{
  QEventLoop loop;
  connect(m_service, SIGNAL(tcpFinish()), &loop, SLOT(quit()));
  qDebug() << "Waiting for interface communication (TestTcpCommunication)";
  loop.exec();
}


QTEST_MAIN(TestTcpServer)
