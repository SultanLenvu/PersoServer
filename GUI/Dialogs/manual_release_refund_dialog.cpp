#include "manual_release_refund_dialog.h"

ManualReleaseRefundDialog::ManualReleaseRefundDialog(QWidget* parent)
    : AbstractInputDialog(parent) {
  setObjectName("ManualReleaseRefundDialog");

  // Считываем размеры дисплея
  DesktopGeometry = QApplication::primaryScreen()->size();

  // Создаем диалоговое окно
  setGeometry(DesktopGeometry.width() * 0.45, DesktopGeometry.height() * 0.45,
              DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1);
  setWindowTitle("Ввод данных");

  create();

  adjustSize();
  setFixedHeight(size().height());

  initMatchTable();
}

ManualReleaseRefundDialog::~ManualReleaseRefundDialog() {}

void ManualReleaseRefundDialog::getData(QHash<QString, QString>* data) const {
  data->insert("table", MatchingTable.value(UnitChoiceComboBox->currentText()));
  data->insert("id", IdLineEdit->text());
}

AbstractInputDialog::InputDialogType ManualReleaseRefundDialog::type() const {
  return ManualReleaseRefund;
}

void ManualReleaseRefundDialog::accept() {
  if (!check()) {
    QMessageBox::critical(this, "Ошибка", "Некорректный ввод данных.", QMessageBox::Ok);
    return;
  }

  QDialog::accept();
}

void ManualReleaseRefundDialog::create() {
  MainLayout = new QGridLayout();
  setLayout(MainLayout);

  UnitChoiceLabel = new QLabel("Единица: ");
  MainLayout->addWidget(UnitChoiceLabel, 0, 0, 1, 1);

  UnitChoiceComboBox = new QComboBox();
  UnitChoiceComboBox->addItem("Транспондер");
  UnitChoiceComboBox->addItem("Бокс");
  UnitChoiceComboBox->addItem("Паллета");
  UnitChoiceComboBox->addItem("Заказ");
  MainLayout->addWidget(UnitChoiceComboBox, 0, 1, 1, 1);

  IdLabel = new QLabel("Идентификатор: ");
  MainLayout->addWidget(IdLabel, 1, 0, 1, 1);

  IdLineEdit = new QLineEdit();
  MainLayout->addWidget(IdLineEdit, 1, 1, 1, 1);

  ButtonLayout = new QHBoxLayout();
  MainLayout->addLayout(ButtonLayout, 2, 0, 1, 2);

  AcceptButton = new QPushButton("Ввод");
  ButtonLayout->addWidget(AcceptButton);
  connect(AcceptButton, &QPushButton::clicked, this, &QDialog::accept);

  RejectButton = new QPushButton("Отмена");
  ButtonLayout->addWidget(RejectButton);
  connect(RejectButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ManualReleaseRefundDialog::initMatchTable() {
  MatchingTable.insert("Транспондер", "transponders");
  MatchingTable.insert("Бокс", "boxes");
  MatchingTable.insert("Паллета", "pallets");
  MatchingTable.insert("Заказ", "orders");
}

bool ManualReleaseRefundDialog::check() const {
  if (IdLineEdit->text().toUInt() == 0) {
    return false;
  }

  return true;
}
