/* RAND.C - Random number generation module. */

/* Copyright (c) 1995-2004 by Radford M. Neal
 *
 * Permission is granted for anyone to copy, use, modify, or distribute this
 * program and accompanying programs and documents for any purpose, provided
 * this copyright notice is retained and prominently displayed, along with
 * a note saying that the original programs are available from Radford Neal's
 * web page, and note is made of any changes made to the programs.  The
 * programs and documents are distributed without any warranty, express or
 * implied.  As the programs were written for research purposes only, they have
 * not been tested to the degree that would be advisable in any important
 * application.  All use of these programs is entirely at the user's own risk.
 */

/* Dependence of the module on a file with "real" random numbers was removed.
 * Instead it uses ROOT TRandom3 generator.
 *
 * -- Andrey Popov
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "rand.h"
#include "../root/CTRandom.h"

/* This module uses the 'drand48' pseudo-random number generator found
   on most Unix systems, the output of which is combined with a file
   of real random numbers.

   Many of the methods used in this module may be found in the following
   reference:

      Devroye, L. (1986) Non-Uniform Random Variate Generation,
        New York: Springer-Verlag.

   The methods used here are not necessarily the fastest available.  They're
   selected to be reasonably fast while also being easy to write.
*/

/* CONSTANT PI.  Defined here if not in <math.h>. */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* TABLES OF REAL RANDOM NUMBERS.  A file of 100000 real random numbers
   (NOT pseudo-random) is used in conjunction with pseudo-random numbers
   for extra insurance.  These are employed in the form of five tables
   of 5000 32-bit integers.

   The file must be located at the path given by RAND_FILE, which should
   be defined on the "cc" command line. */

#define Table_size 5000 /* Number of words in each table */

static int rn[N_tables][Table_size]; /* Random number tables */

/* STATE OF RANDOM NUMBER GENERATOR. */

static int initialized = 0; /* Has module been initialized? */

static rand_state state0; /* Default state structure */

static rand_state *state; /* Pointer to current state */

/* INITIALIZE MODULE.  Sets things up using the default state structure,
   set as if rand_seed had been called with a seed of one. */

static void initialize(void)
{
  int i, j, k, w;
  char b;
  FILE *f;

  // The fragment responsible for accessing RAND_FILE is just commented out. --A.
  if (!initialized)
  {
    /*f = fopen(RAND_FILE,"rb");

    if (f==NULL)
    { fprintf(stderr,"Can't open file of random numbers (%s)\n",RAND_FILE);
      exit(1);
    }

    for (i = 0; i<N_tables; i++)
    { for (j = 0; j<Table_size; j++)
      { w = 0;
        for (k = 0; k<4; k++)
        { if (fread(&b,1,1,f)!=1)
          { fprintf(stderr,"Error reading file of random numbers (%s)\n",
                            RAND_FILE);
            exit(1);
          }
          w = (w<<8) | (b&0xff);
        }
        rn[i][j] = w;
      }
    }*/

    state = &state0;

    initialized = 1;

    rand_seed(1);
  }
}

/* SET CURRENT STATE ACCORDING TO SEED. */

void rand_seed(int seed)
{
  int j;

  if (!initialized)
    initialize();

  state->seed = seed;

  /*state->state48[0] = seed>>16;
  state->state48[1] = seed&0xffff;
  state->state48[2] = rn[0][(seed&0x7fffffff)%Table_size];

  for (j = 0; j<N_tables; j++)
  { state->ptr[j] = seed%Table_size;
    seed /= Table_size;
  }*/

  // Use ROOT random generator instead of the above. --A.
  CTRandom_SetSeed((unsigned int)seed);
}

/* SET STATE STRUCTURE TO USE.  The structure passed must be of the correct
   size and must be set to a valid state.  The only way of obtaining a valid
   state is by copying from the state structure found using rand_get_state. */

void rand_use_state(rand_state *st)
{
  if (!initialized)
    initialize();

  state = st;
}

/* RETURN POINTER TO CURRENT STATE.  A pointer to the state structure being
   used is returned.  */

rand_state *rand_get_state(void)
{
  if (!initialized)
    initialize();

  return state;
}

/* GENERATE RANDOM 31-BIT INTEGER. */

int rand_word(void)
{
  /*int v;
  int j;

  if (!initialized) initialize();

  v = nrand48(state->state48);

  for (j = 0; j<N_tables; j++)
  { v ^= rn[j][state->ptr[j]];
  }

  for (j = 0; j<N_tables && state->ptr[j]==Table_size-1; j++)
  { state->ptr[j] = 0;
  }

  if (j<N_tables)
  { state->ptr[j] += 1;
  }

  return v & 0x7fffffff;*/

  // ROOT random generator is used
  return CTRandom_Integer(0x7fffffff);
}

/* GENERATE UNIFORMLY FROM [0,1). */

double rand_uniform(void)
{
  return (double)rand_word() / (1.0 + (double)0x7fffffff);
}

/* GENERATE UNIFORMLY FORM (0,1). */

double rand_uniopen(void)
{
  return (0.5 + (double)rand_word()) / (1.0 + (double)0x7fffffff);
}

/* GENERATE RANDOM INTEGER FROM 0, 1, ..., (n-1). */

int rand_int(int n)
{
  return (int)(n * rand_uniform());
}

/* GENERATE INTEGER FROM 0, 1, ..., (n-1), WITH GIVEN DISTRIBUTION.  The
   distribution is given by an array of probabilities, which are not
   necessarily normalized, though they must be non-negative, and not all
   zero. */

int rand_pickd(double *p,
               int n)
{
  double t, r;
  int i;

  t = 0;
  for (i = 0; i < n; i++)
  {
    if (p[i] < 0)
      abort();
    t += p[i];
  }

  if (t <= 0)
    abort();

  r = t * rand_uniform();

  for (i = 0; i < n; i++)
  {
    r -= p[i];
    if (r < 0)
      return i;
  }

  /* Return value with non-zero probability if we get here due to roundoff. */

  for (i = 0; i < n; i++)
  {
    if (p[i] > 0)
      return i;
  }

  abort();
}

/* SAME PROCEDURE AS ABOVE, BUT WITH FLOAT ARGUMENT. */

int rand_pickf(float *p,
               int n)
{
  double t, r;
  int i;

  t = 0;
  for (i = 0; i < n; i++)
  {
    if (p[i] <= 0)
      abort();
    t += p[i];
  }

  if (t <= 0)
    abort();

  r = t * rand_uniform();

  for (i = 0; i < n; i++)
  {
    r -= p[i];
    if (r < 0)
      return i;
  }

  /* Return value with non-zero probability if we get here due to roundoff. */

  for (i = 0; i < n; i++)
  {
    if (p[i] > 0)
      return i;
  }

  abort();
}

/* GAUSSIAN GENERATOR.  Done by using the Box-Muller method, but only one
   of the variates is retained (using both would require saving more state).
   See Devroye, p. 235.

   As written, should never deliver exactly zero, which may sometimes be
   helpful. */

double rand_gaussian(void)
{
  double a, b;

  a = rand_uniform();
  b = rand_uniopen();

  return cos(2.0 * M_PI * a) * sqrt(-2.0 * log(b));
}

/* EXPONENTIAL GENERATOR.  See Devroye, p. 29.  Written so as to never
   return exactly zero. */

double rand_exp(void)
{
  return -log(rand_uniopen());
}

/* CAUCHY GENERATOR.  See Devroye, p. 29. */

double rand_cauchy(void)
{
  return tan(M_PI * (rand_uniopen() - 0.5));
}

/* GAMMA GENERATOR.  Generates a positive real number, r, with density
   proportional to r^(a-1) * exp(-r).  See Devroye, p. 410 and p. 420.
   Things are fiddled to avoid ever returning a value that is very near
   zero. */

double rand_gamma(double a)
{
  double b, c, X, Y, Z, U, V, W;

  if (a < 0.00001)
  {
    X = a;
  }

  else if (a <= 1)
  {
    U = rand_uniopen();
    X = rand_gamma(1 + a) * pow(U, 1 / a);
  }

  else if (a < 1.00001)
  {
    X = rand_exp();
  }

  else
  {
    b = a - 1;
    c = 3 * a - 0.75;

    for (;;)
    {
      U = rand_uniopen();
      V = rand_uniopen();

      W = U * (1 - U);
      Y = sqrt(c / W) * (U - 0.5);
      X = b + Y;

      if (X >= 0)
      {
        Z = 64 * W * W * W * V * V;

        if (Z <= 1 - 2 * Y * Y / X || log(Z) <= 2 * (b * log(X / b) - Y))
          break;
      }
    }
  }

  return X < 1e-30 && X < a ? (a < 1e-30 ? a : 1e-30) : X;
}

/* BETA GENERATOR. Generates a real number, r, in (0,1), with density
   proportional to r^(a-1) * (1-r)^(b-1).  Things are fiddled to avoid
   the end-points, and to make the procedure symmetric between a and b. */

double rand_beta(double a,
                 double b)
{
  double x, y, r;

  do
  {
    x = rand_gamma(a);
    y = rand_gamma(b);
    r = 1.0 + x / (x + y);
    r = r - 1.0;
  } while (r <= 0.0 || r >= 1.0);

  return r;
}
