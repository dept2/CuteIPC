#ifndef SIGNALWAITER_H
#define SIGNALWAITER_H

// Qt
#include <QObject>
#include <QEventLoop>
#include <QSignalMapper>
#include <QTimer>
#include <QMap>
#include <QPair>


/**
 * The SignalWaiter class is used to wait for signals emitting (in the eventloop).
 * It is useful to check if signals were emitted necessary times.
 * Use addConnection() method to add the signal to the "spy" list:
 * signalWaiter.addConnection(someObject, SIGNAL(objectSignal()), emittingTimes)
 * Use wait() method to start waiting in the event loop.
 * The method returns false if not all signals were emitted neccessary times during timeout (10secs by default).
 */
class SignalWaiter : public QObject
{
  Q_OBJECT

  public:
    SignalWaiter(QObject* parent = 0);
    ~SignalWaiter();

    void addConnection(QObject* object, const char* signal, int times);
    bool wait(int timeout = 10000);

  public slots:
    void signalEmmited();

  private:
    QEventLoop* m_eventLoop;
    QTimer* m_timer;
    QSignalMapper* m_mapper;
    bool m_allSignalsAreEmitted;

    QMap<QPair<const QObject*, int>, int> m_signalsTimesMap;
};

#endif // SIGNALWAITER_H
