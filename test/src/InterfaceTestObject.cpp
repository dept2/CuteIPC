// Local
#include "InterfaceTestObject.h"

// Qt
#include <QObject>
#include <QDebug>

InterfaceTestObject::InterfaceTestObject(QObject* parent)
    : QObject(parent)
{}


void InterfaceTestObject::interfaceQByteArraySlot(const QByteArray& ba)
{
  m_byteArray = ba;
  emit slotWasCalled(Q_FUNC_INFO);
}


void InterfaceTestObject::interfaceQImageSlot(const QImage& img)
{
  m_image = img;
  emit slotWasCalled(Q_FUNC_INFO);
}


void InterfaceTestObject::interfaceQStringSlot(const QString& str)
{
  m_string = str;
  emit slotWasCalled(Q_FUNC_INFO);
}


void InterfaceTestObject::interfaceIntSlot(int value)
{
  m_int = value;
  emit slotWasCalled(Q_FUNC_INFO);
}


void InterfaceTestObject::interfaceQStringIntSlot(const QString& str, int value)
{
  m_string = str;
  m_int = value;
  emit slotWasCalled(Q_FUNC_INFO);
}


void InterfaceTestObject::interfaceAnotherIntSlot(int value)
{
  m_int = value;
  emit anotherSlotWasCalled(Q_FUNC_INFO);
}


const QByteArray& InterfaceTestObject::getByteArray() const
{
  return m_byteArray;
}


const QImage& InterfaceTestObject::getImage() const
{
  return m_image;
}


const QString& InterfaceTestObject::getString() const
{
  return m_string;
}


int InterfaceTestObject::getInt() const
{
  return m_int;
}


void InterfaceTestObject::emitQByteArraySignal(const QByteArray& ba)
{
  emit interfaceQByteArraySignal(ba);
}

void InterfaceTestObject::emitQImageSignal(const QImage& img)
{
  emit interfaceQImageSignal(img);
}

void InterfaceTestObject::emitQStringSignal(const QString& message)
{
  emit interfaceQStringSignal(message);
}

void InterfaceTestObject::emitIntSignal(int value)
{
  emit interfaceIntSignal(value);
}

void InterfaceTestObject::emitQStringIntSignal(const QString& message, int value)
{
  emit interfaceQStringIntSignal(message, value);
}
