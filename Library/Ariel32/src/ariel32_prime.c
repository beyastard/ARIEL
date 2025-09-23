#include "ariel32_prime.h"

#include <assert.h>


#define prime_table_dimension 80000
#define es_table_dimension 40000

// global variables
uint8_t arz_prime_table[prime_table_dimension];
uint8_t* arz_prime;
uint8_t* arz_prime_low = arz_prime_table;
uint8_t* arz_prime_high = arz_prime_table + prime_table_dimension;

uint32_t arz_es_table[es_table_dimension];
uint32_t* arz_es;
uint32_t* arz_es_low = arz_es_table;
uint32_t* arz_es_high = arz_es_table + es_table_dimension;

arz_t arz_rp_low;
arz_t* arz_ep_low = &arz_rp_low;

arz_t arz_rp_high;
arz_t* arz_ep_high = &arz_rp_high;

int32_t arz_f = 0;


void stop(void)
{
    if (_getch() == tolower('X'))
        exit(EXIT_SUCCESS);
}

// Use the Sieve of Eratosthenes to set up es_tab for
// the next batch of primes, p_lo <= p < p_lo + 64*w.
void arz_get_primes(void)
{
    arz_t rrx = { NULL, 0, 0, 0 };
    arz_t* rx = &rrx;
    uint32_t k_hi = es_table_dimension << 5;

    arz_mov(arz_ep_high, arz_ep_low);
    arz_addk(arz_ep_high, k_hi + k_hi);
    arz_sqrt(rx, arz_ep_high);

    assert(rx->limbs != NULL);
    uint32_t q_hi = arz_wd0(rx) + 2;
    uint32_t f = 0;

    for (arz_es = arz_es_low; arz_es < arz_es_high; arz_es++)
        *arz_es = 0;

    uint32_t q;
    for (q = 3, arz_prime = arz_prime_low; q < q_hi; q += *arz_prime, arz_prime++)
    {
        arz_mov(rx, arz_ep_low);
        arz_modk(rx, q);

        uint32_t k_lo = q - arz_wd0(rx); // k_lo = q - p_lo % q

        if (k_lo == q)
            k_lo = 0;
        else
        {
            if (!(k_lo & 1))
                k_lo >>= 1;
            else
                k_lo = (k_lo + q) >> 1; // k_lo = -p/2 (mod q)
        }

        uint32_t k;
        for (k = k_lo; k < k_hi; k += q)
        {
            *(arz_es_low + (k >> 5)) |= (1 << (k & 31));
            f++;
        }
    }

    printf("ARIEL_PRIME: %d bits set in [", f);
    arz_display(arz_ep_low);
    printf(", ");
    arz_display(arz_ep_high);
    printf("] with sieve limit %d \n", q_hi);
    arz_f = 1;
}

void arz_check_prime_table(void)
{
    uint32_t p, q;
    uint8_t* pr;

    printf("ARIEL_PRIME: Checking pr_tab\n");

    for (pr = arz_prime_low, p = 3, q = 3; pr < arz_prime_high; pr++)
    {
        p += *pr;
        q = _xnext_prime(q);
        if (p != q)
        {
            printf("ARIEL_PRIME: Error at %d in pr_tab: %d should be %d\n", (pr - arz_prime_low), p, q);
            stop();
        }
    }
}

// Load pr_tab with Delta(q) for q = 5, 7, 11, ..., 1020401.
// H. Cohen, CCANT, page 413:
//     Delta(p) < 256 for p < 1872851947 and
//     Delta(p) < 512 for p < 1999066711391.
void arz_load_prime_table(void)
{
    uint32_t q, q0, w, r;
    uint8_t* qr;

    printf("ARIEL_PRIME: Loading prime_table\n");
    for (qr = arz_prime_low, q0 = 3; q0 < 1609; qr++)
    {
        q = _xnext_prime(q0);
        *qr = q - q0;
        q0 = q;
    }

    // Use Sieve of Eratosthenes for larger primes.
    q += 2;
    arz_movk(arz_ep_low, q);
    arz_get_primes();

    for (arz_es = arz_es_low; arz_es < arz_es_high; arz_es++)
    {
        for (r = 0, w = *arz_es; r < 32; r++, w >>= 1, q += 2)
        {
            if (!(w & 1))
            {
                *qr = q - q0;
                qr++;

                if (qr >= arz_prime_high)
                {
                    printf("ARIEL_PRIME: base=3 p[0]=%d p[1]=%d p[2]=%d ... p[%d]=%d p[%d]=%d \n",
                        3 + *arz_prime_low,
                        3 + *arz_prime_low + *(arz_prime_low + 1),
                        3 + *arz_prime_low + *(arz_prime_low + 1) + *(arz_prime_low + 2),
                        arz_prime_high - arz_prime_low - 2, q0,
                        arz_prime_high - arz_prime_low - 1, q);
                    //arz_check_prime_table();
                    return;
                }

                q0 = q;
            }
        }
    }
}

// return next prime after p
void arz_next_prime(arz_t* p)
{
    arz_t rrx = { NULL, 0, 0, 0 };
    arz_t* rx = &rrx;
    
    if (arz_cmpk(p, 1609) <= 0)
    {
        arz_movk(p, _xnext_prime(arz_wd0(p)));
        return;
    }

    if (arz_f == 0)
        arz_load_prime_table();

    arz_addk(p, 1);
    arz_wd0(p) |= 1;

    for (;;)
    {
        if (arz_cmp(p, arz_ep_low) < 0 || arz_cmp(p, arz_ep_high) >= 0)
        {
            arz_mov(arz_ep_low, p);
            arz_get_primes();
        }

        arz_mov(rx, p);
        arz_sub(rx, arz_ep_low);

        assert(rx->limbs != NULL);
        uint32_t k = arz_wd0(rx) >> 1;   // k = bit address in table
        arz_es = arz_es_low + (k >> 5);  // es -> word in table
        uint32_t r = k & 31;             // r = bit in word

        uint32_t w;
        for (w = *arz_es >> r; r < 32; r++, w >>= 1, arz_addk(p, 2))
        {
            if (!(w & 1))
                return;
        }

        for (arz_es++; arz_es < arz_es_high; arz_es++)
        {
            for (r = 0, w = *arz_es; r < 32; r++, w >>= 1, arz_addk(p, 2))
            {
                if (!(w & 1))
                    return;
            }
        }
    }
}
