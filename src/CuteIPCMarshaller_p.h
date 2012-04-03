#ifndef CUTEIPCMARSHALLER_P_H
#define CUTEIPCMARSHALLER_P_H

// Qt
#include <QString>
#include <QByteArray>
#include <QPair>
#include <QGenericArgument>


class CuteIPCMarshaller
{
  public:
    typedef QList<QGenericArgument> Arguments;

    struct Call
    {
      QString first;
      Arguments second;
      QString retType;

      Call(QString method, Arguments arguments, QString retType);
    };
//    typedef QPair<QString, Arguments> Call;

    static QByteArray marshallCall(const QString& method,// QString retType,
        QGenericArgument val0 = QGenericArgument(),
        QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
        QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
        QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
        QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
        QGenericArgument val9 = QGenericArgument(), QString retType = QString());

    static QByteArray marshallReturnedValue(QGenericArgument value);

    static Call demarshallCall(QByteArray call);
    static QGenericArgument demarshallReturnedValue(QByteArray value);

    //service static methods
    static bool marshallArgumentToStream(QGenericArgument value, QDataStream& stream);
    static QGenericArgument demarshallArgumentFromStream(bool& ok, QDataStream& stream);

    static void freeArguments(const Arguments&);
};

#endif // CUTEIPCMARSHALLER_P_H
