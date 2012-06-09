#ifndef CUTEIPCLOOPVECTOR_H
#define CUTEIPCLOOPVECTOR_H

#include <QObject>
#include <QEventLoop>
#include <QList>


class SafeEventLoop : public QEventLoop
{
  Q_OBJECT
  public:
    explicit SafeEventLoop(QObject* parent = 0);
    ~SafeEventLoop();

  signals:
    void mayBeDeleted();

  public slots:
    void exec();
    void endOfExec();
};


class CuteIPCLoopVector : public QObject
{
  Q_OBJECT
  public:
    explicit CuteIPCLoopVector(QObject* sender, const QString& signal, QObject* parent = 0);
    ~CuteIPCLoopVector();

    void append();
    void exec();

  private slots:
    void signalCame();

  signals:
    void loopKiller();

  private:
    QObject* m_sender;
    QString m_signal;
    QList<SafeEventLoop*> m_loopList;
};

#endif // CUTEIPCLOOPVECTOR_H
