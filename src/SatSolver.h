#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <utility>
#include <vector>
#include "SatCnf.h"

std::vector<bool> solve(const SatCnf& sc);

#endif
