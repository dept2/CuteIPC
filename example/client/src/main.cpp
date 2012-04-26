// Qt
#include <QCoreApplication>
#include <QByteArray>
#include <QDebug>
#include <QtGui/QImage>

// CuteIPC
#include <CuteIPCInterface.h>

// Local
#include "TestObjectSlot.h"

int main(int argc, char* argv[])
{
  QCoreApplication a(argc, argv);

  CuteIPCInterface interface;
  if (interface.connectToServer("TestObject"))
  {
    TestObjectSlot* test = new TestObjectSlot();
    interface.remoteSlotConnect(test, SIGNAL(testSignal(QString)), SLOT(foo(QString)));
    interface.remoteConnect(SIGNAL(testSignal2(QString,int)), test, SLOT(debugSlot(QString,int)));
//    interface.remoteConnect(SIGNAL(testSignal(QString)), test, SLOT(debugSlot(QString)));

    QImage testImage("kde-default.png","PNG");

    QByteArray ba(10 * 1024 * 1024, 'H');
    int intval;

    interface.call("bar", Q_RETURN_ARG(int,intval), Q_ARG(QByteArray, ba));
    qDebug() << "FINALLY:" << intval;

    delete test;
    interface.call("bar", Q_RETURN_ARG(int,intval), Q_ARG(QByteArray, ba));
    qDebug() << "FINALLY:" << intval;

    interface.call("imagetest", Q_ARG(QImage,testImage));

//    interface.call("bar", Q_ARG(QByteArray, ba));
//    qDebug() << "FINALLY:" << intval;

//    interface.callAsynchronous("bar", Q_ARG(QByteArray, ba));
  }

  return a.exec();
}
