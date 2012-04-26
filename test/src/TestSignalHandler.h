#ifndef TESTSIGNALHANDLER_H
#define TESTSIGNALHANDLER_H

// Qt
#include <QtTest/QtTest>

// Local
#include <CuteIPCSignalHandler_p.h>
#include <CuteIPCService.h>

class TestSignalHandler : public QObject
{
  Q_OBJECT

  signals:
    void testSignal();
    void testSignal2(QString, bool);

  private slots:
    void initTestCase();
    void testSignalHandlingWithoutArgs();
    void testSignalHandlingWithArgs();
    void testSettingSignalParametersInfo();
    void testListenersManaging();

  private:
    CuteIPCSignalHandler* m_handler;

};

#endif // TESTSIGNALHANDLER_H
