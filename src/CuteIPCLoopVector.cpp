#include "CuteIPCLoopVector.h"
#include <QDebug>


CuteIPCLoopVector::CuteIPCLoopVector(QObject* sender, const QString& signal, QObject* parent)
  : QObject(parent),
    m_sender(sender),
    m_signal(signal)
{
  connect(sender, signal.toAscii(), this, SLOT(signalCame()));
}


CuteIPCLoopVector::~CuteIPCLoopVector()
{
  foreach (QEventLoop* loop, m_loopList)
  {
    loop->quit();
    delete loop;
  }

  m_loopList.clear();
}


void CuteIPCLoopVector::append()
{
  QEventLoop* newLoop = new QEventLoop;
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
    QEventLoop* firstLoop = m_loopList.takeFirst();
    firstLoop->quit();

    // Удаление цикла событий происходит через очередь событий, чтобы дождаться завершения exec() и только
    // потом удалить объект. Иначе метод exec() не успеет закончить работу и в некоторых случаях вызовет падение приложения.
    connect(this, SIGNAL(loopKiller()), firstLoop, SLOT(deleteLater()), Qt::QueuedConnection);
    emit loopKiller();
  }
}
