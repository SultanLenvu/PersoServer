#ifndef PALLETSTICKERPRINTCOMMAND_H
#define PALLETSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"

class PalletStickerPrintCommand : public AbstractClientCommand
{
    Q_OBJECT
public:
    explicit PalletStickerPrintCommand(QObject *parent = nullptr);
};

#endif // PALLETSTICKERPRINTCOMMAND_H
