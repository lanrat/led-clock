#ifndef MUNI_H
#define MUNI_H

struct arrivalETA {
    time_t eta; // epoch
    int route; // 1 = N, 2 = NX, 3 = N_OWL
};

typedef std::vector<arrivalETA> muniETA;

void muniInit();

muniETA muniRun();

void muniCleanup();

#endif