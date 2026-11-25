#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "../../../shared.h"

bool sdManagerBegin();
bool sdManagerRefresh();
StorageStatus sdManagerStatus();

#endif