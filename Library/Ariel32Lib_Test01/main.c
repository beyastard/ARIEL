#include "ariel32.h"

arz_t xreg = { NULL, 0, 0, 0 };
arz_t yreg = { NULL, 0, 0, 0 };
arz_t zreg = { NULL, 0, 0, 0 };
arz_t* rx = &xreg;
arz_t* ry = &yreg;
arz_t* rz = &zreg;

int main()
{
    xprintf("*** ARIEL Library Test ***\n\n");

    arz_movk(rx, 100);  // rx = 100
    arz_mov(ry, rx);    // ry = 100

    arz_addk(ry, 200);  // ry = 100 + 200 = 300

    xprintf("rx + ry =\n");
    xprintf("100 + 200 = ");
    arz_display(ry);

    _ufree(rx);
    _ufree(ry);
    _ufree(rz);

    return 0;
}
