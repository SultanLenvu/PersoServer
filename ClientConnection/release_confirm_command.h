#ifndef TRANSPONDERRELEASECONFIRMCOMMAND_H
#define TRANSPONDERRELEASECONFIRMCOMMAND_H

#include "abstract_client_command.h"

class TransponderReleaseConfirmCommand : public AbstractClientCommand
{
    Q_OBJECT
public:
    explicit TransponderReleaseConfirmCommand(QObject *parent = nullptr);
};

#endif // TRANSPONDERRELEASECONFIRMCOMMAND_H
