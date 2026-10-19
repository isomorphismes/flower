#ifndef FLOWER_POLICY_H
#define FLOWER_POLICY_H
#include "flower.h"
#include <stddef.h>
/* Only policy data crosses this interface. No Flower, hold, clock or step API. */
bool flower_policy_lua(FlowerPolicy *policy,const char *script,char *error,size_t error_size);
#endif
