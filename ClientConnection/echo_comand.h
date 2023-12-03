#ifndef ECHOCOMMAND_H
#define ECHOCOMMAND_H

#include "abstract_client_command.h"

class EchoCommand : public AbstractClientCommand
{
    Q_OBJECT
public:
    explicit EchoCommand(QObject *parent = nullptr);
};

#endif // ECHOCOMMAND_H
