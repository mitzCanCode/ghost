#include "sd_manager.h"

#include <SD.h>

static StorageStatus cardStatus = StorageStatus::NotApplicable;

bool sdManagerRefresh() {
  pinMode(ETH_CS, OUTPUT);
  digitalWrite(ETH_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  bool initialized = SD.begin(SD_CS);
  cardStatus = initialized ? StorageStatus::In : StorageStatus::Out;
  return initialized;
}

bool sdManagerBegin() {
  return sdManagerRefresh();
}

StorageStatus sdManagerStatus() {
  return cardStatus;
}