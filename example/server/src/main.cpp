// Qt
#include <QCoreApplication>

// Local
#include "TestObject.h"


int main(int argc, char* argv[])
{
  QCoreApplication a(argc, argv);

  TestObject obj;
  obj.listen("TestObject");

  return a.exec();
}
