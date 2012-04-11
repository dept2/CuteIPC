// Qt
#include <QCoreApplication>
#include <QByteArray>
#include <QDebug>

// CuteIPC
#include <CuteIPCInterface.h>


int main(int argc, char* argv[])
{
  QCoreApplication a(argc, argv);

  CuteIPCInterface interface;
  if (interface.connectToServer("TestObject"))
  {
    interface.connectRemoteSignal("testSignal(QString)");

    QByteArray ba(10 * 1024 * 1024, 'H');
    int intval;

    interface.call("bar", Q_RETURN_ARG(int,intval), Q_ARG(QByteArray, ba));
    qDebug() << "FINALLY:" << intval;

    interface.call("bar", Q_ARG(QByteArray, ba));
    qDebug() << "FINALLY:" << intval;

    interface.callAsynchronous("bar", Q_ARG(QByteArray, ba));
  }

  return a.exec();
}
