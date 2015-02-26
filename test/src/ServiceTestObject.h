#ifndef SERVICETESTOBJECT_H
#define SERVICETESTOBJECT_H

// Local
#include "CuteIPCService.h"

// Qt
#include <QObject>
#include <QtGui/QImage>

class ServiceTestObject : public CuteIPCService
{
  Q_OBJECT

  public:
    ServiceTestObject(QObject* parent = 0);

    Q_INVOKABLE int testQByteArrayTransfer(const QByteArray&);
    Q_INVOKABLE int testQImageTransfer(const QImage&);
    Q_INVOKABLE int testQStringTransfer(const QString&);
    Q_INVOKABLE QString testQStringTransfer2(const QString&);
    Q_INVOKABLE void testIntTransfer(int);
    Q_INVOKABLE int testQImageGrayScale(const QImage& image);
    Q_INVOKABLE QImage testQImageReturn(const QImage&);

    Q_INVOKABLE int testCallWithRemoteSignal(int);

    const QByteArray& getByteArray() const;
    Q_INVOKABLE QImage getImage() const;
    const QString& getString() const;
    int getInt() const;

  signals:
    void serviceQByteArraySignal(const QByteArray&);
    void serviceQImageSignal(const QImage&);
    void serviceQStringSignal(const QString& message);
    void serviceIntSignal(int);
    void serviceQStringIntSignal(const QString& message, int value);

    void slotWasCalled(const QString& funcInfo);
    void anotherSlotWasCalled(const QString& funcInfo);

    void tcpFinish();

  public slots:
    void serviceQByteArraySlot(const QByteArray&);
    void serviceQImageSlot(const QImage&);
    void serviceQStringSlot(const QString&);
    void serviceIntSlot(int);
    void serviceQStringIntSlot(const QString&, int);

    //to test multiple signal connections
    void serviceAnotherIntSlot(int);

    //emit related signals
    void emitQByteArraySignal(const QByteArray&);
    void emitQImageSignal(const QImage&);
    void emitQStringSignal(const QString& message);
    void emitIntSignal(int);
    void emitQStringIntSignal(const QString& message, int value);
    void emitTcpFinish();

  private:
    QByteArray m_byteArray;
    QImage m_image;
    QString m_string;
    int m_int;
};

#endif // SERVICETESTOBJECT_H
