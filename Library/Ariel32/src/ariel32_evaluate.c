#include "ariel32_evaluate.h"


#define evaluate_dimension 20
#define evaluate_step      32

arz_t eval_rstack[evaluate_dimension];
arz_t* eval_astack[evaluate_dimension];
int32_t eval_ostack[evaluate_dimension];

int32_t evaluate_pointer;
int32_t evaluate_level;
int32_t evaluate_expression;
char* evaluate_error;


// Keyboard was hit. Pause if it was "Q".
void evaluate_break(void)
{
    if (_getch() != tolower('Q'))
        return;

    while (_kbhit())
        (void)_getch();

    printf("Interrupted: Press X to terminate, or ENTER to continue.\n");
    if (_getch() == tolower('X'))
    {
        printf("Program terminated by user.\n");
        exit(EXIT_SUCCESS);
    }
}

void evaluate_factorial(void)
{
    if (arz_sgn(eval_astack[evaluate_pointer]) <= 0)
    {
        arz_movk(eval_astack[evaluate_pointer], 1);
        return;
    }

    _utrim(eval_astack[evaluate_pointer]);

    if (eval_astack[evaluate_pointer]->used_limbs > 1)
    {
        evaluate_error = "Too large!";
        return;
    }

    uint32_t f = arz_wd0(eval_astack[evaluate_pointer]);
    arz_movk(eval_astack[evaluate_pointer], 1);

    for (uint32_t i = 2; i <= f; i++)
    {
        if (_kbhit())
            evaluate_break();

        arz_mulk(eval_astack[evaluate_pointer], i);
    }
}

void evaluate_primorial(void)
{
    if (arz_sgn(eval_astack[evaluate_pointer]) <= 0)
    {
        arz_movk(eval_astack[evaluate_pointer], 1);
        return;
    }

    _utrim(eval_astack[evaluate_pointer]);

    if (eval_astack[evaluate_pointer]->used_limbs > 1)
    {
        evaluate_error = "Too large!";
        return;
    }

    uint32_t f = arz_wd0(eval_astack[evaluate_pointer]);
    arz_movk(eval_astack[evaluate_pointer], 1);

    for (uint32_t i = 2; i <= f; i = _xnext_prime(i))
    {
        if (_kbhit())
            evaluate_break();

        arz_mulk(eval_astack[evaluate_pointer], i);
    }
}

void evaluate_compute(int32_t key)
{
    while (evaluate_pointer > 0 && *evaluate_error == '\0')
    {
        if (_kbhit()) evaluate_break();

        if ((evaluate_level + key) / 4 > eval_ostack[evaluate_pointer - 1] / 4)
            break;

        int32_t k = eval_ostack[evaluate_pointer - 1] % evaluate_step;

        if (k == 12)
            arz_add(eval_astack[evaluate_pointer - 1], eval_astack[evaluate_pointer]);
        else if (k == 13)
            arz_sub(eval_astack[evaluate_pointer - 1], eval_astack[evaluate_pointer]);
        else if (k == 16)
            arz_mul(eval_astack[evaluate_pointer - 1], eval_astack[evaluate_pointer]);
        else if (k == 17)
        {
            if (arz_sgn(eval_astack[evaluate_pointer]) == 0)
                evaluate_error = "Divide by zero!";
            else
            {
                if (arz_sgn(eval_astack[evaluate_pointer]) < 0)
                {
                    arz_neg(eval_astack[evaluate_pointer - 1]);
                    arz_neg(eval_astack[evaluate_pointer]);
                }

                arz_div(eval_astack[evaluate_pointer - 1], eval_astack[evaluate_pointer]);
            }
        }
        else if (k == 18)
        {
            if (arz_sgn(eval_astack[evaluate_pointer]) == 0) evaluate_error = "Divide by zero!";
            else
            {
                if (arz_sgn(eval_astack[evaluate_pointer]) < 0)
                {
                    arz_neg(eval_astack[evaluate_pointer - 1]);
                    arz_neg(eval_astack[evaluate_pointer]);
                }

                arz_mod(eval_astack[evaluate_pointer - 1], eval_astack[evaluate_pointer]);
            }
        }
        else if (k == 20)
        {
            if (arz_sgn(eval_astack[evaluate_pointer]) <= 0)
                arz_movk(eval_astack[evaluate_pointer - 1], 1);
            else
            {
                arz_mov(temp1, eval_astack[evaluate_pointer - 1]);
                arz_exp(eval_astack[evaluate_pointer - 1], temp1, eval_astack[evaluate_pointer]);
            }
        }
        else if (k == 25)
        {
            arz_mov(eval_astack[evaluate_pointer - 1], eval_astack[evaluate_pointer]);
            arz_neg(eval_astack[evaluate_pointer - 1]);
        }
        else if (k == 26)
        {
            if (arz_sgn(eval_astack[evaluate_pointer]) < 0)
                evaluate_error = "Invalid operand!";
            else if (arz_sgn(eval_astack[evaluate_pointer]) == 0)
                arz_movk(eval_astack[evaluate_pointer - 1], 0);
            else
                arz_sqrt(eval_astack[evaluate_pointer - 1], eval_astack[evaluate_pointer]);
        }

        if (evaluate_pointer > 0)
            evaluate_pointer--;
        else
            evaluate_error = "Syntax error!";
    }
}

void evaluate_push(int32_t key)
{
    if (key < 24)
        evaluate_compute(key);
    else
        evaluate_compute(evaluate_step);

    eval_ostack[evaluate_pointer] = evaluate_level + key;

    if (evaluate_pointer + 1 >= evaluate_dimension)
    {
        evaluate_error = "Overflow!";
        return;
    }

    evaluate_pointer++;
    eval_ostack[evaluate_pointer] = 0;
    eval_astack[evaluate_pointer] = &(eval_rstack[evaluate_pointer]);
    arz_movk(eval_astack[evaluate_pointer], 0);
}

// Return the result of evaluating the expression expr.
//
//    Syntax     1  2  3  4  5  6
//
//    1 (        Y  Y  Y  N  N  N
//    2 prefix   Y  Y  Y  N  N  N
//    3 digit    N  N  Y  Y  Y  Y
//    4 suffix   N  N  N  Y  Y  Y
//    5 binary   Y  Y  Y  N  N  N
//    6 )        N  N  N  Y  Y  Y
arz_t* arz_evaluate(char* expr)
{
    char* i;
    int32_t n, t, u;

    eval_astack[0] = &(eval_rstack[0]);
    arz_movk(eval_astack[evaluate_pointer], 0);

    evaluate_level = evaluate_step;
    evaluate_pointer = 0;
    evaluate_expression = 0;
    evaluate_error = "";

    u = 1;
    for (i = expr; *i != '\0'; i++)
    {
        t = u; // t = last type
        if (*evaluate_error != '\0')
            break;

        if (*i <= ' ') // White space
            continue;

        if (*i == ';') // Stop at a semicolon
            break;

        if (*i < '0' || *i > '9')
            evaluate_expression = 1; // It's an expression

        u = 0; // u = this type
        if (t == 1 || t == 2 || t == 5)
        {
            u = 1;
            if (*i == '(') // (
            {
                evaluate_level += evaluate_step;
                continue;
            }

            u = 2;
            if (*i == '+') // Prefix +
            {
                continue;
            }

            if (*i == '-') // Prefix -
            {
                evaluate_push(25);
                continue;
            }

            if (*i == 'q') // Integer square root
            {
                evaluate_push(26);
                continue;
            }
        }

        if (t == 1 || t == 2 || t == 3 || t == 5)
        {
            u = 3;
            if ('0' <= *i && *i <= '9') // Digit
            {
                n = *i - 48; // Convert from ASCII
                arz_mulk(eval_astack[evaluate_pointer], 10);
                arz_addk(eval_astack[evaluate_pointer], n);
                continue;
            }
        }

        if (t == 3 || t == 4 || t == 6)
        {
            u = 4;
            if (*i == '!') // n!
            {
                evaluate_factorial();
                continue;
            }

            if (*i == '#') // n#
            {
                evaluate_primorial();
                continue;
            }

            u = 5;
            if (*i == '+') // Binary +
            {
                evaluate_push(12);
                continue;
            }

            if (*i == '-') // Binary -
            {
                evaluate_push(13);
                continue;
            }

            if (*i == '*')
            {
                evaluate_push(16);
                continue;
            }

            if (*i == '/' || *i == '\\')
            {
                evaluate_push(17);
                continue;
            }

            if (*i == '@')
            {
                evaluate_push(18);
                continue;
            }

            if (*i == '^')
            {
                evaluate_push(20);
                continue;
            }

            u = 6;
            if (*i == ')')  // )
            {
                if (evaluate_level <= evaluate_step)
                    evaluate_error = "Syntax error!";

                evaluate_compute(4);
                evaluate_level -= evaluate_step;
                continue;
            }
        }

        u = 0;
        evaluate_error = "Syntax error!";
    }

    if (evaluate_level > evaluate_step)
        evaluate_error = "Syntax error!";

    evaluate_level = evaluate_step;

    if (*evaluate_error == '\0')
        evaluate_compute(0);

    if (*evaluate_error != '\0')
        arz_movk(eval_astack[0], 0);

    return eval_astack[0];
}
