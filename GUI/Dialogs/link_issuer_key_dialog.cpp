#include "link_issuer_key_dialog.h"

LinkIssuerKeyDialog::LinkIssuerKeyDialog(QWidget* parent)
    : AbstractInputDialog(parent) {
  setObjectName("LinkIssuerKeyDialog");

  // Считываем размеры дисплея
  DesktopGeometry = QApplication::primaryScreen()->size();

  // Создаем диалоговое окно
  setGeometry(DesktopGeometry.width() * 0.45, DesktopGeometry.height() * 0.45,
              DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1);
  setWindowTitle("Параметры связывания");

  create();

  adjustSize();
  setFixedHeight(size().height());

  initMatchTable();
}

LinkIssuerKeyDialog::~LinkIssuerKeyDialog() {}

void LinkIssuerKeyDialog::getData(QHash<QString, QString>* data) const {
  data->insert("issuer_id", IssuerIdLineEdit->text());
  data->insert("key_table",
               MatchingTable.value(KeyChoiceComboBox->currentText()));
  data->insert("key_group_id", KeyGroupLineEdit->text());
}

AbstractInputDialog::InputDialogType LinkIssuerKeyDialog::type() const {
  return ManualReleaseRefund;
}

void LinkIssuerKeyDialog::accept() {
  if (!check()) {
    QMessageBox::critical(this, "Ошибка", "Некорректный ввод данных.", QMessageBox::Ok);
    return;
  }

  QDialog::accept();
}

void LinkIssuerKeyDialog::create() {
  MainLayout = new QGridLayout();
  setLayout(MainLayout);

  IssuerIdLabel = new QLabel("Идентификатор эмитента: ");
  MainLayout->addWidget(IssuerIdLabel, 0, 0, 1, 2);
  IssuerIdLineEdit = new QLineEdit();
  MainLayout->addWidget(IssuerIdLineEdit, 0, 2, 1, 1);

  KeyGroupLabel = new QLabel("Идентификатор ");
  MainLayout->addWidget(KeyGroupLabel, 1, 0, 1, 1);

  KeyChoiceComboBox = new QComboBox();
  KeyChoiceComboBox->addItem("транспортных ключей");
  KeyChoiceComboBox->addItem("коммерческих ключей");
  MainLayout->addWidget(KeyChoiceComboBox, 1, 1, 1, 1);

  KeyGroupLineEdit = new QLineEdit();
  MainLayout->addWidget(KeyGroupLineEdit, 1, 2, 1, 1);

  ButtonLayout = new QHBoxLayout();
  MainLayout->addLayout(ButtonLayout, 2, 0, 1, 3);

  AcceptButton = new QPushButton("Связать");
  ButtonLayout->addWidget(AcceptButton);
  connect(AcceptButton, &QPushButton::clicked, this, &QDialog::accept);

  RejectButton = new QPushButton("Отмена");
  ButtonLayout->addWidget(RejectButton);
  connect(RejectButton, &QPushButton::clicked, this, &QDialog::reject);
}

void LinkIssuerKeyDialog::initMatchTable() {
  MatchingTable.insert("транспортных ключей", "transport_master_keys");
  MatchingTable.insert("коммерческих ключей", "commercial_master_keys");
}

bool LinkIssuerKeyDialog::check() const {
  if (KeyGroupLineEdit->text().toUInt() == 0) {
    return false;
  }

  if (IssuerIdLineEdit->text().toUInt() == 0) {
    return false;
  }

  return true;
}
