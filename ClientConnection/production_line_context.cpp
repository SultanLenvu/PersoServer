#include <QMutexLocker>

#include "production_line_context.h"

ProductionLineContext::ProductionLineContext() {}

ProductionLineContext::~ProductionLineContext() {}

const QString& ProductionLineContext::login() const {
  return Login;
}

void ProductionLineContext::setLogin(const QString& login) {
  Login = login;
}

const QString& ProductionLineContext::password() const {
  return Password;
}

void ProductionLineContext::setPassword(const QString& password) {
  Password = password;
}

bool ProductionLineContext::isActive() const {
  if (ProductionLine.isEmpty()) {
    return false;
  }

  return ProductionLine.get("active") == "true" ? true : false;
}

bool ProductionLineContext::isLaunched() const {
  if (ProductionLine.isEmpty() || Order.isEmpty()) {
    return false;
  }

  return ProductionLine.get("launched") == "true" ? true : false;
}

bool ProductionLineContext::isAuthorized() const {
  if (Password.isEmpty() || Login.isEmpty()) {
    return false;
  }

  return true;
}

bool ProductionLineContext::isInProcess() const {
  if (!isLaunched() || Box.isEmpty()) {
    return false;
  }

  return ProductionLine.get("in_process") == "true" ? true : false;
}

SqlQueryValues& ProductionLineContext::productionLine() {
  return ProductionLine;
}

SqlQueryValues& ProductionLineContext::transponder() {
  return Transponder;
}

SqlQueryValues& ProductionLineContext::box() {
  return Box;
}

SqlQueryValues& ProductionLineContext::pallet() {
  return Pallet;
}

SqlQueryValues& ProductionLineContext::order() {
  return Order;
}

SqlQueryValues& ProductionLineContext::issuer() {
  return Issuer;
}

SqlQueryValues& ProductionLineContext::masterKeys() {
  return MasterKeys;
}

void ProductionLineContext::stash() {
  Stash = std::unique_ptr<ProductionLineContext>(new ProductionLineContext());

  Stash->setLogin(Login);
  Stash->setPassword(Password);

  Stash->ProductionLine = ProductionLine;
  Stash->Transponder = Transponder;
  Stash->Box = Box;
  Stash->Pallet = Pallet;
  Stash->Order = Order;
  Stash->Issuer = Issuer;
  Stash->MasterKeys = MasterKeys;
}

void ProductionLineContext::applyStash() {
  if (!Stash) {
    return;
  }

  Login = Stash->Login;
  Password = Stash->Password;

  ProductionLine = Stash->ProductionLine;
  Transponder = Stash->Transponder;
  Box = Stash->Box;
  Pallet = Stash->Pallet;
  Order = Stash->Order;
  Issuer = Stash->Issuer;
  MasterKeys = Stash->MasterKeys;
}

void ProductionLineContext::generateFirmwareSeed(StringDictionary seed) const {
  // DSRC атрибуты
  seed.insert("personal_account_number",
              Transponder.get("personal_account_number"));
  seed.insert("id", Transponder.get("id"));

  seed.insert("efc_context_mark", Issuer.get("efc_context_mark"));

  seed.insert("manufacturer_id", Order.get("manufacturer_id"));
  seed.insert("equipment_class", Order.get("equipment_class"));
  seed.insert("transponder_model", Order.get("transponder_model"));
  seed.insert("accr_reference", Order.get("accr_reference"));

  // Генерируем дату активации батареи
  QDate date = QDate::currentDate();
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));
  seed.insert("battery_insertation_date", batteryInsertationDate.toUtf8());

  // Мастер ключи безопасности
  seed.insert("accr_key", MasterKeys.get("accr_key"));
  seed.insert("per_key", MasterKeys.get("per_key"));
  seed.insert("au_key1", MasterKeys.get("au_key1"));
  seed.insert("au_key2", MasterKeys.get("au_key2"));
  seed.insert("au_key3", MasterKeys.get("au_key3"));
  seed.insert("au_key4", MasterKeys.get("au_key4"));
  seed.insert("au_key5", MasterKeys.get("au_key5"));
  seed.insert("au_key6", MasterKeys.get("au_key6"));
  seed.insert("au_key7", MasterKeys.get("au_key7"));
  seed.insert("au_key8", MasterKeys.get("au_key8"));
}

void ProductionLineContext::addTransponderDataToJson(QJsonObject& json) const {
  // Данные переносимые без изменений
  json.insert("box_id", Box.get("id"));
  json.insert("pallet_id", Pallet.get("id"));
  json.insert("order_id", Order.get("id"));

  // Удаляем пробелы из названия модели
  QString tempModel = Order.get("transponder_model");
  json.insert("transponder_model", tempModel.remove(" "));

  // Конструируем серийный номер транспондера
  json.insert("sn", generateTransponderSerialNumber(Transponder.get("id")));

  // Вычленяем символы F из personal_account_number
  QString tempPan = Transponder.get("personal_account_number");
  json.insert("pan", tempPan.remove(QChar('F')));

  // Название компании-заказчика
  json.insert("issuer_name", Order.get("name"));
}

void ProductionLineContext::addBoxDataToJson(QJsonObject& json) const {
  json.insert("box_id", Box.get("id"));
  json.insert("box_in_process", Box.get("in_process"));

  json.insert("box_quantity", Box.get("quantity"));
  json.insert("box_assembled_units", Box.get("assembled_units"));

  json.insert("box_assembling_start",
              Box.get("assembling_start").replace("T", " "));
  json.insert("box_assembling_end",
              Box.get("assembling_end").replace("T", " "));

  json.insert("pallet_id", Pallet.get("id"));
  json.insert("production_line_id", ProductionLine.get("id"));
}

QString ProductionLineContext::generateTransponderSerialNumber(
    const QString& id) const {
  // Преобразуем в десятичный формат
  QString manufacturerId =
      QString::number(Order.get("manufacturer_id").toInt(nullptr, 16));

  // Дата сборки
  QStringList tempDate = Box.get("assembling_start").split("T");
  QDate date = QDate::fromString(tempDate.first(), "yyyy-MM-dd");
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));

  // Дополняем серийник до 10 цифр нулями слева
  QString extendedTransponderId = QString("%1").arg(id, 10, QChar('0'));

  // Конструируем серийный номер транспондера
  return QString("%1%2%3").arg(manufacturerId, batteryInsertationDate,
                               extendedTransponderId);
}
