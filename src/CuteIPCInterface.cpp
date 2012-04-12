// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCSlotHandler_p.h"

// Qt
#include <QLocalSocket>
#include <QMetaObject>
#include <QTime>
#include <QEventLoop>
#include <QMetaType>
#include <QTimer>


CuteIPCInterface::CuteIPCInterface(QObject* parent)
  : QObject(parent),
    m_socket(new QLocalSocket(this)),
    m_slotHandler(new CuteIPCSlotHandler(this))
{
}


CuteIPCInterface::~CuteIPCInterface()
{
  delete m_slotHandler;
}


bool CuteIPCInterface::connectToServer(const QString& name)
{
  m_socket->connectToServer(name);
  bool connected = m_socket->waitForConnected(5000);
  if (!connected)
    m_socket->disconnectFromServer();

  if (connected)
  {
    m_connection = new CuteIPCInterfaceConnection(m_socket, this);
    bool ok = connect(m_connection, SIGNAL(invokeRemoteSignal(QString,CuteIPCMessage::Arguments)),
          m_slotHandler, SLOT(invokeRemoteSignal(QString,CuteIPCMessage::Arguments)));
    qDebug() << "Connected:" << connected;
  }

  return connected;
}


bool CuteIPCInterface::connectRemoteSignal(const char* signal)
{
  qDebug() << "";
  qDebug() << "Requesting connection to signal" << signal;

  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, QString::fromAscii(signal));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

//  m_connection->sendCallRequest(request);
  return sendSynchronousRequest(request);
//  return false;
}


bool CuteIPCInterface::remoteConnect(const char* signal, QObject* object, const char* slot)
{
  QString signalSignature = QString::fromAscii(signal).mid(1);
  QString slotSignature = QString::fromAscii(slot).mid(1);

  if (!checkConnectCorrection(signalSignature, object, slotSignature))
    return false;

  m_slotHandler->registerConnection(signalSignature, object, slotSignature);
  return true;
}


bool CuteIPCInterface::checkConnectCorrection(const QString& signal, const QObject* object, const QString& slot)
{
  //check signal and slot existing
  if (!object || object->metaObject()->indexOfSlot(slot.toAscii()) == -1)
  {
    qDebug() << "incorrect connect reciever";
    return false;
  }

  if (!QMetaObject::checkConnectArgs(signal.toAscii(), slot.toAscii()))
  {
    qDebug() << "incompatible signatures" << signal << slot;
    return false;
  }

  qDebug() << "Requesting connection to signal" << signal;
  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, signal);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);
  bool ok = sendSynchronousRequest(request);
  return ok;
}


bool CuteIPCInterface::call(const QString& method, QGenericReturnArgument ret, QGenericArgument val0,
                            QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4,
                            QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8,
                            QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9, QString::fromLatin1(ret.name()));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  m_connection->setReturnedObject(ret);
  qDebug() << "Trying to call" << method;

  return sendSynchronousRequest(request);
}


bool CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "Trying to call" << method;

  return sendSynchronousRequest(request);
}


bool CuteIPCInterface::sendSynchronousRequest(const QByteArray& request)
{
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);

  QEventLoop loop;
  connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
  loop.exec();

  qDebug() << "call was finished. stop local event loop...";

  return m_connection->lastCallSuccessful();
}


void CuteIPCInterface::callNoReply(const QString& method, QGenericArgument val0, QGenericArgument val1,
                                        QGenericArgument val2, QGenericArgument val3, QGenericArgument val4,
                                        QGenericArgument val5, QGenericArgument val6, QGenericArgument val7,
                                        QGenericArgument val8, QGenericArgument val9)
{
  qDebug() << "";
  qDebug() << "Before marshalling:" << QTime::currentTime().toString("hh:mm:ss.zzz");

  CuteIPCMessage message(CuteIPCMessage::MessageCallWithoutReturn,
                         method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  qDebug() << "Call (asynchronously)" << method;
  qDebug() << "(Method serialized into" << request.size() << "bytes)";
  m_connection->sendCallRequest(request);
}


QString CuteIPCInterface::lastError() const
{
  return m_connection->lastError();
}


void CuteIPCInterface::debugSlot(QString str, int val)
{
  qDebug() << Q_FUNC_INFO << str << val;
//  qDebug() << "Start timer in invoked slot...";
//  QTimer timer;
//  QEventLoop loop(this);
//  connect(&timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
//  timer.start( 5000 ); // wait at least 100 ms between polling
//  loop.exec();
//  qDebug() << "Stop timer in invoked slot";
}
