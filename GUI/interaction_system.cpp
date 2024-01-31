#include "interaction_system.h"

#include "General/definitions.h"

InteractionSystem::InteractionSystem(QWidget* parent) : QWidget(parent) {
  setObjectName("InteractionSystem");
  loadSettings();

  createProgressDialog();

  // Создаем таймеры
  createTimers();

  CurrentDialog = nullptr;
}

InteractionSystem* InteractionSystem::instance() {
  static InteractionSystem interactor(nullptr);
  return &interactor;
}

void InteractionSystem::generateMessage(const QString& data) {
  QMessageBox::information(this, "Сообщение", data, QMessageBox::Ok);
}

void InteractionSystem::getMasterPassword(QString& pass) {
  pass = QInputDialog::getText(
      this, "Мастер доступ", "Введите пароль:", QLineEdit::Normal, "", nullptr);
}

void InteractionSystem::generateErrorMessage(const QString& text) {
  QMessageBox::critical(this, "Ошибка", text, QMessageBox::Ok);
}

void InteractionSystem::startOperationProgressDialog(
    const QString& operationName) {
  QSettings settings;

  // Открываем окно загрузки
  ProgressDialog.setValue(0);
  ProgressDialog.show();

  //  Настраиваем и запускаем таймер для измерения квантов времени
  uint64_t operationDuration =
      settings.value(QString("duration_of_operations/") + operationName)
          .toInt();
  uint32_t operationQuantDuration = operationDuration / 100;
  operationQuantDuration += 5;
  sendLog(QString("Длительность кванта операции: %1.")
              .arg(QString::number(operationQuantDuration)));
  ODQTimer->setInterval(operationQuantDuration);
  ODQTimer->start();

  // Запускаем таймер для контроля максимальной длительности операции
  //  ODTimer->start();

  // Запускаем измеритель длительности операции
  ODMeter->start();
}

void InteractionSystem::finishOperationProgressDialog(
    const QString& operationName) {
  QSettings settings;

  // Измеряем и сохраняем длительность операции
  uint64_t duration = ODMeter->elapsed();
  sendLog(QString("Длительность операции: %1.").arg(QString::number(duration)));
  settings.setValue(QString("duration_of_operations/") + operationName,
                    QVariant::fromValue(duration));

  // Останавливаем таймер для контроля максимальной длительности операции
  //  ODTimer->stop();

  // Останавливаем для измерения кванта длительности операции
  ODQTimer->stop();

  // Закрываем окно
  ProgressDialog.close();
}

void InteractionSystem::applySettings() {
  sendLog("Применение новых настроек. ");
  loadSettings();
}

/*
 * Приватные методы
 */

void InteractionSystem::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
}

void InteractionSystem::sendLog(const QString& log) {
  if (LogEnable) {
    emit logging(QString("%1 - %2").arg(objectName(), log));
  }
}

void InteractionSystem::createProgressDialog() {
  ProgressDialog.setLabelText("Выполнение операции...");
  ProgressDialog.setMinimum(0);
  ProgressDialog.setMaximum(100);
  ProgressDialog.setCancelButton(nullptr);
  ProgressDialog.setWindowModality(Qt::ApplicationModal);
  ProgressDialog.setAutoClose(false);
  ProgressDialog.close();
}

void InteractionSystem::createTimers() {
  // Таймер, отслеживающий длительность выполняющихся операций
  ODTimer = new QTimer(this);
  ODTimer->setInterval(SERVER_MANAGER_OPERATION_MAX_DURATION);
  connect(ODTimer, &QTimer::timeout, this,
          &InteractionSystem::ODTimerTimeout_slot);
  connect(ODTimer, &QTimer::timeout, ODTimer, &QTimer::stop);

  // Таймер для измерения длительности операции
  ODMeter = new QElapsedTimer();

  // Таймер, отслеживающий квант длительности операции
  ODQTimer = new QTimer(this);
  connect(ODQTimer, &QTimer::timeout, this,
          &InteractionSystem::ODQTimerTimeout_slot);
}

void InteractionSystem::processCurrentDialog(QHash<QString, QString>* param) {
  if (CurrentDialog->exec() == QDialog::Rejected) {
    return;
  }

  CurrentDialog->getData(param);
}

void InteractionSystem::progressDialogCanceled_slot() {
  ProgressDialog.close();

  emit abortCurrentOperation();
}

void InteractionSystem::ODTimerTimeout_slot() {
  sendLog("Операция выполняется слишком долго. Сброс. ");
  generateErrorMessage("Операция выполняется слишком долго. Сброс. ");
}

void InteractionSystem::ODQTimerTimeout_slot() {
  if (ProgressDialog.value() < 100) {
    ProgressDialog.setValue(ProgressDialog.value() + 1);
  } else {
    ODQTimer->stop();
  }
}

void InteractionSystem::invalidInput_slot() {
  generateErrorMessage("Некорректный ввод данных.");
}
