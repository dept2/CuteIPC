#ifndef INTERFACETESTOBJECT_H
#define INTERFACETESTOBJECT_H

// Qt
#include <QObject>
#include <QtGui/QImage>

class InterfaceTestObject : public QObject
{
  Q_OBJECT

  public:
    InterfaceTestObject(QObject* parent = 0);
    ~InterfaceTestObject() {}

    const QByteArray& getByteArray() const;
    const QImage& getImage() const;
    const QString& getString() const;
    int getInt() const;

  signals:
    void interfaceQByteArraySignal(const QByteArray&);
    void interfaceQImageSignal(const QImage&);
    void interfaceQStringSignal(const QString& msg);
    void interfaceIntSignal(int);
    void interfaceQStringIntSignal(const QString& message, int value);

    void interfaceAnotherIntSignal(int);

    void slotWasCalled(const QString& funcInfo);
    void anotherSlotWasCalled(const QString& funcInfo);

  public slots:
    void interfaceQByteArraySlot(const QByteArray&);
    void interfaceQImageSlot(const QImage&);
    void interfaceQStringSlot(const QString&);
    void interfaceIntSlot(int);
    void interfaceQStringIntSlot(const QString&, int);

    //to test multiple signal connections
    void interfaceAnotherIntSlot(int);

    //emit related signals
    void emitQByteArraySignal(const QByteArray&);
    void emitQImageSignal(const QImage&);
    void emitQStringSignal(const QString& message);
    void emitIntSignal(int);
    void emitQStringIntSignal(const QString& message, int value);

  private:
    QByteArray m_byteArray;
    QImage m_image;
    QString m_string;
    int m_int;
};

#endif // INTERFACETESTOBJECT_H
