#ifndef ROLLBACKCOMMAND_H
#define ROLLBACKCOMMAND_H

#include "abstract_client_command.h"

class RollbackCommand : public AbstractClientCommand
{
    Q_OBJECT
public:
    explicit RollbackCommand(QObject *parent = nullptr);
};

#endif // ROLLBACKCOMMAND_H
