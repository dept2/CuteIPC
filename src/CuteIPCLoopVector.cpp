// Local
#include "CuteIPCLoopVector.h"


CuteIPCLoopVector::CuteIPCLoopVector(QObject* sender, const QString& signal, QObject* parent)
  : QObject(parent),
    m_sender(sender),
    m_signal(signal)
{
  connect(sender, signal.toAscii(), this, SLOT(signalCame()));
}


CuteIPCLoopVector::~CuteIPCLoopVector()
{
  foreach (SafeEventLoop* loop, m_loopList)
    loop->quit();

  m_loopList.clear();
}


void CuteIPCLoopVector::append()
{
  SafeEventLoop* newLoop = new SafeEventLoop;
  m_loopList.append(newLoop);
}


void CuteIPCLoopVector::exec()
{
  if (!m_loopList.isEmpty())
    m_loopList.last()->exec();
}


void CuteIPCLoopVector::signalCame()
{
  if (!m_loopList.isEmpty())
  {
    SafeEventLoop* firstLoop = m_loopList.takeFirst();
    firstLoop->quit();
  }
}


SafeEventLoop::SafeEventLoop(QObject *parent)
  : QEventLoop(parent)
{
  connect(this, SIGNAL(mayBeDeleted()), SLOT(endOfExec()));
}


SafeEventLoop::~SafeEventLoop()
{}


void SafeEventLoop::exec()
{
  QEventLoop::exec();
  emit mayBeDeleted();
}


void SafeEventLoop::endOfExec()
{
  if (!this->isRunning())
    this->deleteLater();
}
