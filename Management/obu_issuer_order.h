#ifndef OBUINITIALIZATOR_H
#define OBUINITIALIZATOR_H

#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QObject>

class ObuIssuerOrder : public QObject {
  Q_OBJECT
 private:
  QFile* PanFile;
  QString IssuerName;
  bool FullPersonalization;
  uint32_t ObuQuantity;
  QDate ProductionStartDate;

  QString CurrentPan;

 public:
  explicit ObuIssuerOrder(QObject* parent,
                          const QString& issuerName,
                          uint32_t obuQuantity);

  bool setFullPersonalization(const QString& panFilePath);

  QString* issuerName(void);
  bool fullPersonalization(void) const;
  uint32_t obuQuantity(void) const;
  QDate* productionStartDate(void);
  QString* currentPan(void);
  bool nextPan(void);

 signals:
};

#endif  // OBUINITIALIZATOR_H
