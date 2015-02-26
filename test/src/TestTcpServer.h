#ifndef TESTTCPSERVER_H
#define TESTTCPSERVER_H

// Qt
#include <QtTest/QtTest>

// Local
class ServiceTestObject;
class CuteIPCInterface;

class TestTcpServer : public QObject
{
  Q_OBJECT

  private slots:
    // init, cleanup
    void init();
    void cleanup();

    // basic communications tests
    void generalTest();

  private:
    ServiceTestObject* m_service;
};

#endif //TESTTCPSERVER_H
