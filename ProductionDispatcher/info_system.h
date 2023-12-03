#ifndef INFOSYSTEM_H
#define INFOSYSTEM_H

#include "abstract_info_system.h"

class InfoSystem : public AbstractInfoSystem
{
    Q_OBJECT
public:
    explicit InfoSystem(QObject *parent = nullptr);
};

#endif // INFOSYSTEM_H
