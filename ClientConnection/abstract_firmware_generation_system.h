#ifndef ABSTRACTFIRMWAREGENERATIONSYSTEM_H
#define ABSTRACTFIRMWAREGENERATIONSYSTEM_H

#include <QObject>

class AbstractFirmwareGenerationSystem : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFirmwareGenerationSystem(QObject *parent = nullptr);

signals:

};

#endif // ABSTRACTFIRMWAREGENERATIONSYSTEM_H
