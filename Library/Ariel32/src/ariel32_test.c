#include "ariel32_test.h"


int32_t terminate_program = 0;

void halt(void)
{
    while (_kbhit())
        (void)_getch();

    printf("ARIEL_TEST stopped: Press X to terminate, anything else to continue.\n");
    if (_getch() == 'X')
    {
        printf("ARIEL_TEST terminated by user.\n");
        exit(EXIT_SUCCESS);
    }

    printf("ARIEL_TEST restarted...\n");
    terminate_program = 0;
}

int32_t test_quit(void)
{
    if (!_kbhit())
        return 0;

    if (_getch() != tolower('Q'))
        return 0;

    terminate_program = 1;

    return 1;
}

// Print "Checksum failed" message if a != b
// Print "Fail" / "Pass" message if &a = &b
// Add (b - a)^2 to chk
void test_check_register(arz_t* a, arz_t* b, char* id, int32_t ik)
{
    arz_t rchk = { NULL, 0, 0, 0 };
    arz_t rchx = { NULL, 0, 0, 0 };
    arz_t* chk = &rchk;
    arz_t* chx = &rchx;
    int32_t checknum = 0;

    if (test_quit())
        halt();

    if (chk->limbs == NULL)
        arz_movk(chk, 0);

    if (a == b)
    {
        if (arz_sgn(chk))
            printf("Fail!\n");
        else
            printf("Pass!\n");
    }
    else
    {
        checknum++;

        arz_mov(chx, a);
        arz_sub(chx, b);

        if (checkFFTerror && maxFFTerror > 0.25)
        {
            printf("Checksum id %s [%d] number %d FFT error:", id, ik, checknum);
            printf("maxFFTerror = %g \n", maxFFTerror);

            halt();
            maxFFTerror = 0;
        }

        if (CheckStop * arz_sgn(chx))
        {
            printf("Checksum id %s [%d] number %d failed:", id, ik, checknum);
            printf("\n  Actual value:  ");
            arz_display(a);
            printf("\n  Correct value: ");
            arz_display(b);
            printf("\n  Difference:    ");
            arz_display(chx);
            printf("\n");

            halt();
        }
        else
        {
            if (CheckLog)
            {
                printf("Checksum id %s [%d] number %d Okay\n", id, ik, checknum);

                if (CheckStop > 1)
                    halt();
            }
        }

        arz_squ(chx);
        arz_add(chk, chx);
    }
}

void test_check_digits(arz_t* a, char* s, char* id, int32_t ik)
{
    arz_t rchs = { NULL, 0, 0, 0 };
    arz_t* chs = &rchs;
    arz_movd(chs, s);
    test_check_register(a, chs, id, ik);
}
