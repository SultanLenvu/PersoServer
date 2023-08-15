#include "perso_init_dialog.h"

PersoInitDialog::PersoInitDialog(QWidget* parent) : QDialog(parent) {
  // Считываем размеры дисплея
  DesktopGeometry = QApplication::desktop()->screenGeometry();

  // Создаем диалоговое окно
  setGeometry(DesktopGeometry.width() * 0.5, DesktopGeometry.height() * 0.5,
              DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1);
  setWindowTitle("Персонализация");

  MainLayout = new QVBoxLayout();
  setLayout(MainLayout);

  MainLabel = new QLabel("Введите данные для персонализации");
  MainLayout->addWidget(MainLabel);

  PersoInitDataInput = new QPlainTextEdit();
  MainLayout->addWidget(PersoInitDataInput);

  OkButton = new QPushButton("Начать");
  MainLayout->addWidget(OkButton);
  connect(OkButton, &QPushButton::clicked, this, &QDialog::accept);

  CancelButton = new QPushButton("Отмена");
  MainLayout->addWidget(CancelButton);
  connect(CancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString PersoInitDialog::getData(void) {
  return PersoInitDataInput->toPlainText();
}
