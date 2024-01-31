#ifndef ABSTRACTPERSOSERVER_H
#define ABSTRACTPERSOSERVER_H

#include <QObject>

class AbstractPersoServer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPersoServer(QObject *parent = nullptr);

signals:

};

#endif // ABSTRACTPERSOSERVER_H
