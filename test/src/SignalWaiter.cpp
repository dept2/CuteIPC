// Local
#include "SignalWaiter.h"

// Qt
#include <QEventLoop>
#include <QTimer>
#include <QSignalMapper>
#include <QMetaMethod>


SignalWaiter::SignalWaiter(QObject* parent)
  : QObject(parent),
    m_eventLoop(new QEventLoop(this)),
    m_timer(new QTimer(this)),
    m_mapper(new QSignalMapper(this)),
    m_allSignalsAreEmitted(false)
{
  connect(m_timer, SIGNAL(timeout()), m_eventLoop, SLOT(quit()));
  m_timer->setSingleShot(true);
}


SignalWaiter::~SignalWaiter()
{}


void SignalWaiter::addConnection(QObject* object, const char* signal, int times)
{
  connect(object, signal, SLOT(signalEmmited()), Qt::UniqueConnection);

  int signalIndex = object->metaObject()->indexOfSignal(QString::fromAscii(signal).mid(1).toAscii());
  QPair<const QObject*, int> pair(object, signalIndex);
  m_signalsTimesMap.insert(pair, times);
  m_allSignalsAreEmitted = false;
}


bool SignalWaiter::wait(int timeout)
{
  m_timer->stop();
  m_timer->start(timeout);

  if (!m_allSignalsAreEmitted) {
    m_eventLoop->exec();
  }

  // Возвращаем true, если из eventLoop'а мы вышли не по таймауту, а за счет метода signalEmmited()
  if (m_timer->isActive())
    return true;
  return false;
}


void SignalWaiter::signalEmmited()
{
  QPair<const QObject*, int> pair(sender(), senderSignalIndex());

  int count = m_signalsTimesMap.value(pair);
  if (count == 0)
    return;

  count--;
  if (count == 0)
    m_signalsTimesMap.remove(pair);
  else
    m_signalsTimesMap.insert(pair, count);

  // Дождались вызова всех сигналов
  if (m_signalsTimesMap.isEmpty()) {
    m_allSignalsAreEmitted = true;
    m_eventLoop->quit();
  }
}

