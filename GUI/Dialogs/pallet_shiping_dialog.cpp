#include "pallet_shiping_dialog.h"

PalletShippingDialog::PalletShippingDialog(QWidget* parent)
    : AbstractInputDialog(parent) {
  setObjectName("PalletShippingDialog");

  setWindowTitle("Отгрузка");
  DesktopGeometry = QApplication::primaryScreen()->size();
  setGeometry(DesktopGeometry.width() * 0.5, DesktopGeometry.height() * 0.5,
              DesktopGeometry.width() * 0.2, DesktopGeometry.height() * 0.05);

  create();

  adjustSize();
  setFixedHeight(size().height());
}

PalletShippingDialog::~PalletShippingDialog() {}

void PalletShippingDialog::getData(QHash<QString, QString>* data) const {
  data->insert("first_pallet_id", FirstPalletId->text());
  data->insert("last_pallet_id", LastPalletId->text());
}

AbstractInputDialog::InputDialogType PalletShippingDialog::type() const {
  return PalletShipping;
}

void PalletShippingDialog::accept() {
  if (!check()) {
    QMessageBox::critical(this, "Ошибка", "Некорректный ввод данных.", QMessageBox::Ok);
    return;
  }

  QDialog::accept();
}

void PalletShippingDialog::create() {
  MainLayout = new QGridLayout(this);
  setLayout(MainLayout);

  PalletLabel1 = new QLabel("Паллеты с");
  MainLayout->addWidget(PalletLabel1, 0, 0, 1, 1);

  FirstPalletId = new QLineEdit();
  MainLayout->addWidget(FirstPalletId, 0, 1, 1, 1);

  PalletLabel2 = new QLabel("по");
  MainLayout->addWidget(PalletLabel2, 0, 2, 1, 1);

  LastPalletId = new QLineEdit();
  MainLayout->addWidget(LastPalletId, 0, 3, 1, 1);

  ButtonLayout = new QHBoxLayout();
  MainLayout->addLayout(ButtonLayout, 1, 0, 1, 4);

  AcceptButton = new QPushButton("Отгрузить");
  ButtonLayout->addWidget(AcceptButton);
  connect(AcceptButton, &QPushButton::clicked, this, &QDialog::accept);

  RejectButton = new QPushButton("Отмена");
  ButtonLayout->addWidget(RejectButton);
  connect(RejectButton, &QPushButton::clicked, this, &QDialog::reject);
}

bool PalletShippingDialog::check() const {
  if ((FirstPalletId->text().toUInt() == 0) ||
      (LastPalletId->text().toUInt() == 0)) {
    return false;
  }

  if (FirstPalletId->text().toInt() > LastPalletId->text().toInt()) {
    return false;
  }

  return true;
}
