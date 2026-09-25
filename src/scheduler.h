#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "parser.h"

//ejecuta las actividades del DAG respetando el limite de concurrencia K
void run_scheduler(TASK *tasks, int total, int K);

#endif