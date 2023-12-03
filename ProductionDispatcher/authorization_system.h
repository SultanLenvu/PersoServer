#ifndef AUTHORIZATIONSYSTEM_H
#define AUTHORIZATIONSYSTEM_H

#include "abstract_authorization_system.h"

class AuthorizationSystem : public AbstractAuthorizationSystem
{
    Q_OBJECT
public:
    explicit AuthorizationSystem(QObject *parent = nullptr);
};

#endif // AUTHORIZATIONSYSTEM_H
