#ifndef CUTEIPCLOOPVECTOR_H
#define CUTEIPCLOOPVECTOR_H

#include <QObject>
#include <QEventLoop>
#include <QList>

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

  private:
    QObject* m_sender;
    QString m_signal;
    QList<QEventLoop*> m_loopList;
};

#endif // CUTEIPCLOOPVECTOR_H
