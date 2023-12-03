#ifndef GENERALPRODUCTIONDISPATCHER_H
#define GENERALPRODUCTIONDISPATCHER_H

#include "abstract_production_dispatcher.h"

class GeneralProductionDispatcher : public AbstractProductionDispatcher
{
    Q_OBJECT
public:
    explicit GeneralProductionDispatcher(QObject *parent = nullptr);
};

#endif // GENERALPRODUCTIONDISPATCHER_H
