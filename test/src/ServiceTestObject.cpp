// Local
#include "ServiceTestObject.h"

// Qt
#include <QDebug>
#include <QtGui/QImage>


ServiceTestObject::ServiceTestObject(QObject* parent)
: CuteIPCService(parent)
{}


int ServiceTestObject::testQByteArrayTransfer(const QByteArray& ba)
{
  m_byteArray = ba;
  return ba.size();
}


int ServiceTestObject::testQImageTransfer(const QImage& img)
{
  m_image = img;
  return img.byteCount();
}


int ServiceTestObject::testQStringTransfer(const QString& string)
{
  m_string = string;
  return string.size();
}


void ServiceTestObject::testIntTransfer(int value)
{
  m_int = value;
}


int ServiceTestObject::testQImageGreyScale(const QImage& image)
{
  QImage originImage(":/images/finger.png");
  if (originImage.width() != image.width() || originImage.height() != image.height())
    return -1;

  for (int y = 0; y < originImage.height(); ++y)
    for (int x = 0; x < originImage.width(); ++x)
      if (qRed(originImage.pixel(x, y)) != qRed(image.pixel(x, y)) || qGreen(originImage.pixel(x, y)) != qGreen(image.pixel(x, y))
          || qBlue(originImage.pixel(x, y)) != qBlue(image.pixel(x, y)))
        return -1;

  return image.byteCount();
}


void ServiceTestObject::serviceQByteArraySlot(const QByteArray& ba)
{
  this->testQByteArrayTransfer(ba);
  emit slotWasCalled(Q_FUNC_INFO);
}


void ServiceTestObject::serviceQImageSlot(const QImage& img)
{
  this->testQImageTransfer(img);
  emit slotWasCalled(Q_FUNC_INFO);
}


void ServiceTestObject::serviceQStringSlot(const QString& str)
{
  this->testQStringTransfer(str);
  emit slotWasCalled(Q_FUNC_INFO);
}


void ServiceTestObject::serviceIntSlot(int value)
{
  this->testIntTransfer(value);
  emit slotWasCalled(Q_FUNC_INFO);
}


void ServiceTestObject::serviceQStringIntSlot(const QString& str, int value)
{
  this->testQStringTransfer(str);
  this->testIntTransfer(value);
  emit slotWasCalled(Q_FUNC_INFO);
}


void ServiceTestObject::serviceAnotherIntSlot(int value)
{
  this->testIntTransfer(value);
  emit anotherSlotWasCalled(Q_FUNC_INFO);
}


const QByteArray& ServiceTestObject::getByteArray() const
{
  return m_byteArray;
}


const QImage& ServiceTestObject::getImage() const
{
  return m_image;
}


const QString& ServiceTestObject::getString() const
{
  return m_string;
}


int ServiceTestObject::getInt() const
{
  return m_int;
}

void ServiceTestObject::emitQByteArraySignal(const QByteArray& ba)
{
  emit serviceQByteArraySignal(ba);
}

void ServiceTestObject::emitQImageSignal(const QImage& img)
{
  emit serviceQImageSignal(img);
}

void ServiceTestObject::emitQStringSignal(const QString& message)
{
  emit serviceQStringSignal(message);
}

void ServiceTestObject::emitIntSignal(int value)
{
  emit serviceIntSignal(value);
}

void ServiceTestObject::emitQStringIntSignal(const QString &message, int value)
{
  emit serviceQStringIntSignal(message, value);
}
