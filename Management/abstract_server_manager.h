#ifndef ABSTRACTSERVERMANAGER_H
#define ABSTRACTSERVERMANAGER_H

#include <QObject>

class AbstractServerManager : public QObject
{
    Q_OBJECT
public:
    explicit AbstractServerManager(QObject *parent = nullptr);

signals:

};

#endif // ABSTRACTSERVERMANAGER_H
