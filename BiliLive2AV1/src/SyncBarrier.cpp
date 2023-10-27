#include "SyncBarrier.h"

Dispath dispath_f(SyncBarrier *syb)
{
    fprintf(stderr, "dispath calledl \n");
    // sleep(3);
    // Dispath aka(syb);
    fprintf(stderr, "ready to destry \n");
    return Dispath();
    // return Dispath(syb);
    // return aka;
}
Dispath SyncBarrier::dispath()
{
    // sleep(4);
    Dispath ana(this);
    // fprintf(stderr, "call : sss and ID is  %zu\n",(size_t)this->AtomicInt.load());
    fflush(stderr);
    // sleep(4);
    
    // dispath_f(this);
    return ana;
}
