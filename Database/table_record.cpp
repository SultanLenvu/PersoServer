#include "table_record.h"

UniversalTableRecord::UniversalTableRecord(QObject* parent) : QObject(parent) {}

UniversalTableRecord::~UniversalTableRecord() {
  clear();
}

void UniversalTableRecord::addAttribute(const QString& name,
                                        const QString& value) {
  Attributes.append(new QPair<QString, QString>(name, value));
}

const QString& UniversalTableRecord::attributeValue(const uint32_t i) const {
  return Attributes.at(i)->second;
}

const QString& UniversalTableRecord::attributeName(const uint32_t i) const {
  return Attributes.at(i)->first;
}

void UniversalTableRecord::clear() {
  for (int32_t i = 0; i < Attributes.size(); i++) {
    delete Attributes.at(i);
  }
  Attributes.clear();
}
