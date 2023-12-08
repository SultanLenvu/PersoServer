#ifndef GETCURRENTCONTEXTCOMMAND_H
#define GETCURRENTCONTEXTCOMMAND_H

#include "abstract_client_command.h"

class GetCurrentContextCommand : public AbstractClientCommand {
  Q_OBJECT
 public:
  GetCurrentContextCommand();

 private:
  Q_DISABLE_COPY_MOVE(GetCurrentContextCommand)
};

#endif  // GETCURRENTCONTEXTCOMMAND_H
