
#ifndef WLD_H
#define WLD_H

#include "define.h"
#include "structs.h"
#include "container.h"

int wld_open(Wld* wld, SimpleBuffer* file);
void wld_close(Wld* wld);

#endif/*WLD_H*/
