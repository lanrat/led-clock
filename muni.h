#ifndef MUNI_H
#define MUNI_H

struct  muniETA {
    std::vector<int> N;
    std::vector<int> NX;
};

void muniInit();

muniETA muniRun();

void muniCleanup();

#endif