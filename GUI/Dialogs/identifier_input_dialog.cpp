#include "idetifier_input_dialog.h"

IdentifierInputDialog::IdentifierInputDialog(QWidget* parent)
    : AbstractInputDialog(parent) {
  setObjectName("IdentifierInputDialog");

  // Считываем размеры дисплея
  DesktopGeometry = QApplication::primaryScreen()->size();

  // Создаем диалоговое окно
  setGeometry(DesktopGeometry.width() * 0.45, DesktopGeometry.height() * 0.45,
              DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1);
  setWindowTitle("Ввод данных");

  create();

  adjustSize();
  setFixedHeight(size().height());
}

IdentifierInputDialog::~IdentifierInputDialog() {}

void IdentifierInputDialog::getData(QHash<QString, QString>* data) const {
  data->insert("id", InputData->text());
}

AbstractInputDialog::InputDialogType IdentifierInputDialog::type() const {
  return IdentifierInput;
}

void IdentifierInputDialog::accept() {
  if (!check()) {
    QMessageBox::critical(this, "Ошибка", "Некорректный ввод данных.",
                          QMessageBox::Ok);
    return;
  }

  QDialog::accept();
}

void IdentifierInputDialog::create() {
  MainLayout = new QGridLayout();
  setLayout(MainLayout);

  MainLabel = new QLabel("Идентификатор: ");
  MainLayout->addWidget(MainLabel, 0, 0, 1, 1);

  InputData = new QLineEdit();
  MainLayout->addWidget(InputData, 0, 1, 1, 1);

  ButtonLayout = new QHBoxLayout();
  MainLayout->addLayout(ButtonLayout, 1, 0, 1, 2);

  AcceptButton = new QPushButton("Ввод");
  ButtonLayout->addWidget(AcceptButton);
  connect(AcceptButton, &QPushButton::clicked, this, &QDialog::accept);

  RejectButton = new QPushButton("Отмена");
  ButtonLayout->addWidget(RejectButton);
  connect(RejectButton, &QPushButton::clicked, this, &QDialog::reject);
}

bool IdentifierInputDialog::check() const {
  if (InputData->text().toUInt() == 0) {
    return false;
  }

  return true;
}
