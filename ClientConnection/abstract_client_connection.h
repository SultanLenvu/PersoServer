#ifndef ABSTARCTPERSOCLIENT_H
#define ABSTARCTPERSOCLIENT_H

#include <QObject>

class AbstarctPersoClient : public QObject
{
    Q_OBJECT
public:
    explicit AbstarctPersoClient(QObject *parent = nullptr);

signals:

};

#endif // ABSTARCTPERSOCLIENT_H
