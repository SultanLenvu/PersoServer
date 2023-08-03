#include "perso_manager.h"

/* Диспетчер DSRC  */
//==================================================================================
PersoManager::PersoManager(QObject* parent) : QObject(parent) {
  DatabaseController = new PostgresController(this);
  connect(DatabaseController, &DatabaseControllerInterface::logging, this,
          &PersoManager::proxyLogging);
}

PersoManager::~PersoManager() {}

void PersoManager::connectDatabase() {
  DatabaseController->connect();
}

void PersoManager::disconnectDatabase() {
  DatabaseController->disconnect();
}

void PersoManager::performCustomSqlRequest(const QString& req) {
  clearDatabaseLastResponse();

  DatabaseController->execCustomRequest(req, &DatabaseLastResponse);

  logDatabaseReponse();
}

void PersoManager::applySettings(UserSettings* settings) const {}

/*
 * Приватные слоты
 */

void PersoManager::delayEnd(void) {
  ReadyFlag = true;
}

void PersoManager::proxyLogging(const QString& log) {
  if (sender()->objectName() == QString("PostgresController"))
    emit logging(QString("Postgres сontroller - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void PersoManager::clearDatabaseLastResponse() {
  for (int32_t i = 0; i < DatabaseLastResponse.size(); i++)
    DatabaseLastResponse[i].clear();

  DatabaseLastResponse.clear();
}

void PersoManager::logDatabaseReponse(void) {
  for (int32_t i = 0; i < DatabaseLastResponse.size(); i++)
    for (int32_t j = 0; j < DatabaseLastResponse[i].size(); j++)
      emit logging(DatabaseLastResponse[i][j]);
}
//==================================================================================
