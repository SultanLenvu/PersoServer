#ifndef ABSTRACTCLIENTCOMMAND_H
#define ABSTRACTCLIENTCOMMAND_H

#include <QObject>

class AbstractClientCommand : public QObject
{
    Q_OBJECT
public:
    explicit AbstractClientCommand(QObject *parent = nullptr);

signals:

};

#endif // ABSTRACTCLIENTCOMMAND_H
