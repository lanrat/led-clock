#ifndef MUNI_H
#define MUNI_H

struct  muniETA {
    std::vector<int> N; // includes N_OWL
    std::vector<int> NX;
};

void muniInit();

muniETA muniRun();

void muniCleanup();

#endif