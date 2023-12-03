#ifndef TRANSPONDERRERELEASECOMMAND_H
#define TRANSPONDERRERELEASECOMMAND_H

#include "abstract_client_command.h"

class TransponderRereleaseCommand : public AbstractClientCommand
{
    Q_OBJECT
public:
    explicit TransponderRereleaseCommand(QObject *parent = nullptr);
};

#endif // TRANSPONDERRERELEASECOMMAND_H
