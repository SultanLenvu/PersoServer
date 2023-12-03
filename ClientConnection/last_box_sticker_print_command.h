#ifndef LASTBOXSTICKERPRINTCOMMAND_H
#define LASTBOXSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"

class LastBoxStickerPrintCommand : public AbstractClientCommand
{
    Q_OBJECT
public:
    explicit LastBoxStickerPrintCommand(QObject *parent = nullptr);
};

#endif // LASTBOXSTICKERPRINTCOMMAND_H
