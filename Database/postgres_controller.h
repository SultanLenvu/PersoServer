#ifndef POSTGRESCONTROLLER_H
#define POSTGRESCONTROLLER_H

#include "database_controller.h"

class PostgresController : public IDatabaseController {
  Q_OBJECT

 private:
  QString ConnectionName;
  QHostAddress HostAddress;
  uint32_t Port;
  QString DatabaseName;
  QString UserName;
  QString Password;

 public:
  explicit PostgresController(QObject* parent, const QString& connectionName);
  ~PostgresController();

 public:
  // IDatabaseController interface
  virtual bool connect(void) override;
  virtual void disconnect(void) override;
  virtual bool isConnected(void) override;

  virtual bool openTransaction(void) const override;
  virtual bool closeTransaction(void) const override;
  virtual bool abortTransaction(void) const override;

  virtual bool execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) const override;
  virtual bool getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseTableModel* buffer) const override;
  virtual bool clearTable(const QString& tableName) const override;

  virtual bool addRecord(const QString& tableName,
                         QHash<QString, QString>& record) const override;

  virtual bool getRecordById(const QString& tableName,
                             QHash<QString, QString>& record) const override;
  virtual bool getRecordByPart(const QString& tableName,
                               QHash<QString, QString>& record,
                               bool order = true) const override;
  virtual bool getLastRecord(const QString& tableName,
                             QHash<QString, QString>& record) const override;

  virtual bool getMergedRecordById(
      const QStringList& tables,
      const QStringList& foreignKeys,
      QHash<QString, QString>& record) const override;
  virtual bool getMergedRecordByPart(
      const QStringList& tables,
      const QStringList& foreignKeys,
      QHash<QString, QString>& record) const override;

  bool getAllMergedRecords(
      const QStringList& tableNames,
      const QStringList& foreignKeys,
      const QHash<QString, QString>& searchValue,
      std::vector<std::unique_ptr<QHash<QString, QString>>>& records) const;

  virtual bool updateRecordById(const QString& tableName,
                                QHash<QString, QString>& record) const override;
  bool updateAllRecordsByPart(const QString& tableName,
                              QHash<QString, QString>& conditions,
                              QHash<QString, QString>& newValues) const;
  virtual bool removeRecordById(const QString& tableName,
                                const uint32_t id) const override;
  virtual bool removeLastRecordById(const QString& tableName) const override;
  virtual bool removeLastRecordByCondition(
      const QString& tableName,
      QHash<QString, QString>& condition) const override;

  virtual void applySettings() override;

 private:
  Q_DISABLE_COPY_MOVE(PostgresController);
  void loadSettings(void);
  void createDatabaseConnection(void);
  void convertResponseToBuffer(QSqlQuery& request,
                               DatabaseTableModel* buffer) const;
  void convertResponseToHash(QSqlQuery& request,
                             QHash<QString, QString>& record) const;
  void convertResponseToTable(
      QSqlQuery& request,
      std::vector<std::unique_ptr<QHash<QString, QString>>>& table) const;
};

#endif  // POSTGRESCONTROLLER_H
