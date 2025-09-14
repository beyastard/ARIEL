#include "ariel32.h"

arz_t xreg = { NULL, 0, 0, 0 };
arz_t yreg = { NULL, 0, 0, 0 };
arz_t* rx = &xreg;
arz_t* ry = &yreg;

int main()
{
    xprintf("*** ARIEL Library Test ***");

    arz_movk(rx, 100);
    arz_mov(ry, rx);

    _ufree(rx);
    _ufree(ry);

    return 0;
}
