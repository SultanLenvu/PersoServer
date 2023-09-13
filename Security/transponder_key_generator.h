#ifndef TRANSPONDER_KEY_GENERATOR_H
#define TRANSPONDER_KEY_GENERATOR_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>

#include "General/definitions.h"
#include "des.h"

enum class KeyType { Master, Common };

enum class KeyReference {
  AuthenticationKey1 = 111,
  AuthenticationKey2 = 112,
  AuthenticationKey3 = 113,
  AuthenticationKey4 = 114,
  AuthenticationKey5 = 115,
  AuthenticationKey6 = 116,
  AuthenticationKey7 = 117,
  AuthenticationKey8 = 118,
  AccessCredentialsKey = 120,
  PersonalizationKey = 121
};

enum class KeyIndex {
  AuthenticationKey1 = 0,
  AuthenticationKey2 = 1,
  AuthenticationKey3 = 2,
  AuthenticationKey4 = 3,
  AuthenticationKey5 = 4,
  AuthenticationKey6 = 5,
  AuthenticationKey7 = 6,
  AuthenticationKey8 = 7,
  AccessCredentialsKey = 8,
  PersonalizationKey = 9
};

enum class KeyGroupType { Unknown, Transport, Commercial, Auto, System };

class TransponderKeyGenerator : public QAbstractListModel {
  Q_OBJECT
 protected:
  QVector<QByteArray> MasterKeys;
  QByteArray accrKeysSeed;
  QByteArray auKeysSeed;

 public:
  TransponderKeyGenerator();
  ~TransponderKeyGenerator();

  void generate(QMap<QString, QString>* transponderData);

 private:
  void init(QMap<QString, QString>* transponderData);
};

#endif  // TRANSPONDER_KEY_GENERATOR_H
