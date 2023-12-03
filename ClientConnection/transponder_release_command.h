#ifndef TRANSPONDERRELEASECOMMAND_H
#define TRANSPONDERRELEASECOMMAND_H

#include "abstract_client_command.h"

class TransponderReleaseCommand : public AbstractClientCommand
{
    Q_OBJECT
public:
    explicit TransponderReleaseCommand(QObject *parent = nullptr);
};

#endif // TRANSPONDERRELEASECOMMAND_H
