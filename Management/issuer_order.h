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
  explicit IssuerOrder(QObject* parent);

  void setIssuerName(const QString& newIssuerName);
  void setTransponderQuantity(uint32_t newTransponderQuantity);
  bool setFullPersonalization(const QString& panFilePath);

  const QString& issuerName(void);
  bool fullPersonalization(void) const;
  uint32_t transponderQuantity(void) const;
  const QDate& productionStartDate(void);
  const QString& currentPan(void);
  bool nextPan(void);

 signals:
};

#endif  // OBUINITIALIZATOR_H
