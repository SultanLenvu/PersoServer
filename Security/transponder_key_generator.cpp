#include "transponder_key_generator.h"

/* Инициирующее значение ключа безопасности определенного транспондера */
//==================================================================================

// CommonSecurityKeyInitializer::CommonSecurityKeyInitializer(
//    DsrcAttribute* accrRef) {
//  if (accrRef->name() == "AccrReference")
//    for (uint32_t i = 0; i < 4; i++)
//      Value.append(*accrRef->value());
//}

// CommonSecurityKeyInitializer::CommonSecurityKeyInitializer(DsrcAttribute*
// pan,
//                                                           DsrcAttribute* ecm)
//                                                           {
//  if ((pan->name() == "PaymentMeans") && (ecm->name() == "EfcContextMark")) {
//    QByteArray compactPan;

//    for (uint32_t i = 0; i < 4; i++)
//      compactPan.append(pan->value()->at(i) ^ pan->value()->at(i + 4));

//    Value.append(compactPan);
//    Value.append(ecm->value()->at(0));
//    Value.append(ecm->value()->at(1));
//    Value.append(ecm->value()->at(2));
//    Value.append(static_cast<uint8_t>(0x00));
//  } else {
//    Value.append(static_cast<uint8_t>(0x00));
//  }
//}

// CommonSecurityKeyInitializer::~CommonSecurityKeyInitializer() {}

// QByteArray* CommonSecurityKeyInitializer::value(void) {
//  return &Value;
//}

//==================================================================================
/* Модель группы ключей безопасности транспондера */
//==================================================================================

TransponderKeyGenerator::TransponderKeyGenerator() {}

TransponderKeyGenerator::~TransponderKeyGenerator() {}

//==================================================================================

void TransponderKeyGenerator::generate(
    QMap<QString, QString>* transponderData) {}
