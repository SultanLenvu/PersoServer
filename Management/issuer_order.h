#ifndef OBUINITIALIZATOR_H
#define OBUINITIALIZATOR_H

#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QObject>

class IssuerOrder : public QObject {
  Q_OBJECT
 private:
  QFile* PanFile;
  QString IssuerName;
  bool FullPersonalization;
  uint32_t TransponderQuantity;
  QDate ProductionStartDate;

  QString CurrentPan;

 public:
  explicit IssuerOrder(const QString& issuerName,
                       uint32_t TransponderQuantity,
                       QObject* parent);

  bool setFullPersonalization(const QString& panFilePath);

  QString* issuerName(void);
  bool fullPersonalization(void) const;
  uint32_t transponderQuantity(void) const;
  QDate* productionStartDate(void);
  QString* currentPan(void);
  bool nextPan(void);

 signals:
};

#endif  // OBUINITIALIZATOR_H
