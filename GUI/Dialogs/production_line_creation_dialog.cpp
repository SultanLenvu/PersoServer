#include "production_line_creation_dialog.h"
#include "General/definitions.h"

ProductionLineCreationDialog::ProductionLineCreationDialog(QWidget* parent)
    : AbstractInputDialog(parent) {
  setObjectName("PanInputDialog");

  // Считываем размеры дисплея
  DesktopGeometry = QApplication::primaryScreen()->size();

  // Создаем диалоговое окно
  setGeometry(DesktopGeometry.width() * 0.4, DesktopGeometry.height() * 0.45,
              DesktopGeometry.width() * 0.2, DesktopGeometry.height() * 0.1);
  setWindowTitle("Cоздание производственной линии");

  create();

  adjustSize();
  setFixedHeight(size().height());
}

ProductionLineCreationDialog::~ProductionLineCreationDialog() {}

void ProductionLineCreationDialog::getData(
    QHash<QString, QString>* data) const {
  data->insert("login", LoginLineEdit->text());
  data->insert("password", PasswordLineEdit->text());
  data->insert("name", NameLineEdit->text());
  data->insert("surname", SurnameLineEdit->text());
}

AbstractInputDialog::InputDialogType ProductionLineCreationDialog::type()
    const {
  return ProductionLineCreation;
}

void ProductionLineCreationDialog::accept() {
  if (!check()) {
    QMessageBox::critical(this, "Ошибка", "Некорректный ввод данных.", QMessageBox::Ok);
    return;
  }

  QDialog::accept();
}

void ProductionLineCreationDialog::create() {
  MainLayout = new QGridLayout();
  setLayout(MainLayout);

  LoginLabel = new QLabel("Логин: ");
  MainLayout->addWidget(LoginLabel, 0, 0, 1, 1);
  LoginLineEdit = new QLineEdit();
  LoginLineEdit->setMaxLength(PL_LOGIN_MAX_LENGHT);
  MainLayout->addWidget(LoginLineEdit, 0, 1, 1, 1);

  PasswordLabel = new QLabel("Пароль: ");
  MainLayout->addWidget(PasswordLabel, 1, 0, 1, 1);
  PasswordLineEdit = new QLineEdit();
  PasswordLineEdit->setMaxLength(PL_PASSWORD_MAX_LENGHT);
  MainLayout->addWidget(PasswordLineEdit, 1, 1, 1, 1);

  NameLabel = new QLabel("Имя сборщика: ");
  MainLayout->addWidget(NameLabel, 2, 0, 1, 1);
  NameLineEdit = new QLineEdit();
  NameLineEdit->setMaxLength(PL_NAME_MAX_LENGHT);
  MainLayout->addWidget(NameLineEdit, 2, 1, 1, 1);

  SurnameLabel = new QLabel("Фамилия сборщика: ");
  MainLayout->addWidget(SurnameLabel, 3, 0, 1, 1);
  SurnameLineEdit = new QLineEdit();
  SurnameLineEdit->setMaxLength(PL_SURNAME_MAX_LENGHT);
  MainLayout->addWidget(SurnameLineEdit, 3, 1, 1, 1);

  VerticalSpacer =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  MainLayout->addItem(VerticalSpacer, 4, 0, 1, 2);

  ButtonLayout = new QHBoxLayout();
  MainLayout->addLayout(ButtonLayout, 5, 0, 1, 2);

  AcceptButton = new QPushButton("Ввод");
  MainLayout->addWidget(AcceptButton);
  connect(AcceptButton, &QPushButton::clicked, this, &QDialog::accept);

  RejectButton = new QPushButton("Отмена");
  MainLayout->addWidget(RejectButton);
  connect(RejectButton, &QPushButton::clicked, this, &QDialog::reject);
}

bool ProductionLineCreationDialog::check() const {
  return true;
}
