// Qt
#include <QCoreApplication>
#include <QByteArray>

// CuteIPC
#include <CuteIPCInterface.h>


int main(int argc, char* argv[])
{
  QCoreApplication a(argc, argv);

  CuteIPCInterface interface;
  interface.connectToServer("TestObject");

//  interface.call("foo", Q_ARG(QString, QString::fromUtf8("Hello world")));
  QByteArray ba(10 * 1024 * 1024, 'H');
  interface.call("bar", Q_ARG(QByteArray, ba));

  return a.exec();
}
