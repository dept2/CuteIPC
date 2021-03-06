// Local
#include "TestSignalHandler.h"

// CuteIPC
#include <CuteIPCMessage_p.h>
#include <CuteIPCMarshaller_p.h>
#include <CuteIPCSignalHandler_p.h>
#include <CuteIPCService.h>
#include <CuteIPCServiceConnection_p.h>

// Qt
#include <QtTest/QtTest>
#include <QtGui/QImage>
#include <QLocalSocket>

void TestSignalHandler::initTestCase()
{
  m_handler = 0;
}


void TestSignalHandler::testSignalHandlingWithoutArgs()
{
  QString signalSignature = "testSignal()";
  m_handler = new CuteIPCSignalHandler(signalSignature, this);

  bool result = QMetaObject::connect(this, this->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toLatin1())),
                                     m_handler, m_handler->metaObject()->indexOfSlot("relaySlot()"));
  QCOMPARE(result, true);

  QSignalSpy spy(m_handler, SIGNAL(signalCaptured(QByteArray)));

  emit testSignal();

  QCOMPARE(spy.count(), 1);
  QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE(arguments.size(), 1);

  QByteArray serializedMessage = arguments.at(0).toByteArray();
  CuteIPCMessage message = CuteIPCMarshaller::demarshallMessage(serializedMessage);
  QCOMPARE(message.messageType(), CuteIPCMessage::MessageSignal);
  QCOMPARE(message.method(), QString(signalSignature));
  QVERIFY(message.returnType().isEmpty());
  QVERIFY(message.arguments().isEmpty());

  delete m_handler;
}


void TestSignalHandler::testSignalHandlingWithArgs()
{
  QString signalSignature = "testSignal2(QString, bool)";
  QString testString = "test";
  bool testBool = true;

  m_handler = new CuteIPCSignalHandler(signalSignature, this);

  bool result = QMetaObject::connect(this, this->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toLatin1())),
                                     m_handler, m_handler->metaObject()->indexOfSlot("relaySlot()"));
  QCOMPARE(result, true);

  QSignalSpy spy(m_handler, SIGNAL(signalCaptured(QByteArray)));

  emit testSignal2(testString, testBool);

  QCOMPARE(spy.count(), 1);
  QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE(arguments.size(), 1);

  QByteArray serializedMessage = arguments.at(0).toByteArray();
  CuteIPCMessage message = CuteIPCMarshaller::demarshallMessage(serializedMessage);
  QCOMPARE(message.messageType(), CuteIPCMessage::MessageSignal);
  QCOMPARE(message.method(), QString(signalSignature));
  QVERIFY(message.returnType().isEmpty());
  QVERIFY(message.arguments().size() == 2);

  QCOMPARE(message.arguments().at(0).name(), "QString");
  QCOMPARE(message.arguments().at(1).name(), "bool");
  QCOMPARE(*reinterpret_cast<QString*>(message.arguments().at(0).data()), testString);
  QCOMPARE(*reinterpret_cast<bool*>(message.arguments().at(1).data()), true);

  delete m_handler;
}


void TestSignalHandler::testSettingSignalParametersInfo()
{
  TestSignalHandler* newObject = new TestSignalHandler();
  QString signatureToSend = "newSignal2(QString,bool)";
  QString signatureToCatch = "testSignal2(QString,bool)";

  m_handler = new CuteIPCSignalHandler(signatureToSend, newObject);
  m_handler->setSignalParametersInfo(this, signatureToCatch);

  bool result = QMetaObject::connect(this, this->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signatureToCatch.toLatin1())),
                                     m_handler, m_handler->metaObject()->indexOfSlot("relaySlot()"));
  QCOMPARE(result, true);

  QSignalSpy spy(m_handler, SIGNAL(signalCaptured(QByteArray)));

  emit testSignal2("test", true);

  QCOMPARE(spy.count(), 1);
  QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE(arguments.size(), 1);

  QByteArray serializedMessage = arguments.at(0).toByteArray();
  CuteIPCMessage message = CuteIPCMarshaller::demarshallMessage(serializedMessage);

  QCOMPARE(message.method(), QString(signatureToSend));
  QVERIFY(message.arguments().size() == 2);
  QCOMPARE(message.arguments().at(0).name(), "QString");
  QCOMPARE(message.arguments().at(1).name(), "bool");

  delete m_handler;
  delete newObject;
}


void TestSignalHandler::testListenersManaging()
{
  //TODO: need to spy for a slot (CuteIPCServiceConnection::sendSignal(QByteArray)).
  QLocalSocket* socket1 = new QLocalSocket(this);
  QLocalSocket* socket2 = new QLocalSocket(this);
  CuteIPCService* service = new CuteIPCService();

  service->listen("TestSocket");
  socket1->connectToServer("TestSocket");
  socket2->connectToServer("TestSocket");
  socket1->open(QIODevice::ReadWrite);
  socket2->open(QIODevice::ReadWrite);

  CuteIPCServiceConnection* listener1 = new CuteIPCServiceConnection(socket1, service);
  CuteIPCServiceConnection* listener2 = new CuteIPCServiceConnection(socket2, service);

  QString signalSignature = "testSignal2(QString, bool)";
  m_handler = new CuteIPCSignalHandler(signalSignature, service);

  bool result = QMetaObject::connect(this, this->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toLatin1())),
                                m_handler, m_handler->metaObject()->indexOfSlot("relaySlot()"));
  QCOMPARE(result, true);

  m_handler->addListener(listener1);
  m_handler->addListener(listener2);

  QSignalSpy spy(m_handler, SIGNAL(signalCaptured(QByteArray)));
  QSignalSpy spyForListener1Destroyed(listener1, SIGNAL(destroyed(QObject*)));
  QSignalSpy spyForListener2Destroyed(listener2, SIGNAL(destroyed(QObject*)));
  QSignalSpy spyForHandlerDestroyed(m_handler, SIGNAL(destroyed(QString)));

  emit testSignal2("test", true);
  QCOMPARE(spy.count(), 1);

  delete listener1;
  QCOMPARE(spyForListener1Destroyed.count(), 1);
  QCOMPARE(spyForListener2Destroyed.count(), 0);
  QCOMPARE(spyForHandlerDestroyed.count(),0);
  delete listener2;
  QCOMPARE(spyForListener1Destroyed.count(), 1);
  QCOMPARE(spyForListener2Destroyed.count(), 1);
  QCOMPARE(spyForHandlerDestroyed.count(),1);
  QList<QVariant> arguments = spyForHandlerDestroyed.takeFirst();
  QCOMPARE(arguments.size(), 1);
  QCOMPARE(arguments.at(0).toString(), signalSignature);

  delete service;
  delete socket1;
  delete socket2;
}

QTEST_MAIN(TestSignalHandler)
