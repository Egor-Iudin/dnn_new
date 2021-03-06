
/* MC-HYBRID.C - Procedure for performing Hybrid Monte Carlo updates. */

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "misc.h"
#include "rand.h"
#include "log.h"
#include "mc.h"

/* PERFORM HYBRID MONTE CARLO OPERATION. */

void mc_hybrid(mc_dynamic_state *ds, /* State to update */
               mc_iter *it,          /* Description of this iteration */
               mc_traj *tj,          /* Trajectory specification */
               int steps,            /* Total number of steps to do */
               int window,           /* Window size to use */
               int jump,             /* Number of steps in each jump */
               double temper_factor, /* Temper factor, zero if not tempering */
               int N_quantities,     /* Number of quantities to plot, -1 for tt plot */
               mc_value *q_save,     /* Place to save original q */
               mc_value *p_save,     /* Place to save original p */
               mc_value *q_asv,      /* Place to save q values for accept state */
               mc_value *p_asv,      /* Place to save p values for accept state */
               mc_value *q_rsv,      /* Place to save q values for reject state */
               mc_value *p_rsv       /* Place to save p values for reject state */
)
{
  double rej_pot_energy, rej_kinetic_energy, rej_free_energy;
  double acc_pot_energy, acc_kinetic_energy, acc_free_energy;
  double old_pot_energy, old_kinetic_energy;

  int have_rej, rej_point;
  int have_acc, acc_point;

  int n, k, dir, jmps;
  double H, U;

  double hiybryd_error;

  float *ErrorList;
  float *NumbersList;

  if (steps % jump != 0)
    abort();

  jmps = steps / jump;

  // for (k = 0; k<ds->dim; k++)
  //            { printf("ds-p[%d]= %f \n",k, ds->p[k]) ;
  //		printf("ds-q[%d]= %f \n",k, ds->q[k]) ;
  //
  // ds->q[k] ;
  //          }
  //	getchar();

  ErrorList = (float *)chk_alloc(1, 100 * sizeof(double));
  NumbersList = (float *)chk_alloc(1, 100 * sizeof(int));
  int mc_app_energy_callNumber = 0;
  FILE *fp;
  fp = fopen(ds->netFileName, "a+");
  fprintf(fp, "%s", "StartHybryb \n");

  fclose(fp);

  //  printf("Error In mc_app_energy = %f \n",sqrt(error_compute/eventsNumber));
  /* Decide on window offset, and save start state if we'll have to restore. */

  it->window_offset = rand_int(window);

  if (it->window_offset > 0)
  {
    mc_value_copy(q_save, ds->q, ds->dim);
    mc_value_copy(p_save, ds->p, ds->dim);

    if (!ds->know_pot)
    {
      printf("mc_app_energy 1 \n");
      mc_app_energy(ds, 1, 1, &ds->pot_energy, 0);
      ds->know_pot = 1;
    }

    if (!ds->know_kinetic)
    {
      ds->kinetic_energy = mc_kinetic_energy(ds);
      ds->know_kinetic = 1;
    }

    old_pot_energy = ds->pot_energy;
    old_kinetic_energy = ds->kinetic_energy;
  }

  /* Set up for travelling about trajectory. */

  mc_traj_init(tj, it);
  mc_traj_permute();

  have_rej = have_acc = 0;
  n = it->window_offset;
  dir = -1;

  /* Loop that looks at one new state each iteration, which may possibly
     be in the accept and/or reject windows. */

  if (temper_factor != 0 && N_quantities < 0)
  {
    printf("\n");
  }
  int i = 0;
  /*  while (dir==-1 || n!=jmps)*/

  /* #pragma omp parallel for private(ds,it,tj,q_save,p_save,q_asv,p_asv,q_rsv,p_rsv,n,k,dir,i,steps,temper_factor,N_quantities,window,jump,H,U,have_rej, rej_point,have_acc, acc_point,rej_pot_energy, rej_kinetic_energy, rej_free_energy,acc_pot_energy, acc_kinetic_energy, acc_free_energy,old_pot_energy, old_kinetic_energy)
   */
  // getchar();
  while (dir == -1 || n != jmps)
  {
    /* Restore if next state should be original start state. */

    // printf ("While in hybrid Start \n");
    // printf ("While in hybrid1  %d  %d %d  %d  1\n",i,n,dir,jmps);
    i++;
    if (dir == -1 && n == 0)
    {
      /* printf ("While in hybrid1  %d 2\n",i);*/
      if (it->window_offset > 0)
      {
        // printf ("hybryd visihislenie 2 \n");
        mc_value_copy(ds->q, q_save, ds->dim);
        mc_value_copy(ds->p, p_save, ds->dim);

        ds->pot_energy = old_pot_energy;
        ds->kinetic_energy = old_kinetic_energy;

        ds->know_pot = 1;
        ds->know_kinetic = 1;
        ds->know_grad = 0;
      }

      n = it->window_offset;
      dir = 1;
    }

    else
    {
      /* printf ("While in hybrid1 %d 3 \n",i);*/
      /* Do tempering in second half if appropriate. */

      if (temper_factor != 0 && n <= jmps - window && 2 * n > jmps)
      {
        // printf("ds-dim= %d",ds->dim);

        for (k = 0; k < ds->dim; k++)
        {
          printf("hybryd visihislenie 1 \n");
          ds->p[k] /= temper_factor;
        }
        if (N_quantities < 0)
        {
          printf("%6d %20.10f\n", jmps - n + 1,
                 mc_kinetic_energy(ds) * (temper_factor * temper_factor - 1));
          printf("%6d %20.10f\n", jmps - n,
                 mc_kinetic_energy(ds) * (temper_factor * temper_factor - 1));
        }
      }

      /* Next state is found by following trajectory for 'jump' steps. */
      // printf ("hybryd visihislenie 11 \n");
      n += dir;
      // printf("mc_trajectory 1 \n");
      mc_trajectory(ds, dir * jump, n < window || n > jmps - window);
      // printf("dir = %d\n   jmps  = %d \n  n= %d",dir,jmps,n);
      /* Do tempering in first half if appropriate. */

      if (temper_factor != 0 && n >= window && 2 * n < jmps)
      {
        /*printf ("While in hybrid1 %d 4 \n",i);*/
        if (N_quantities < 0)
        {
          printf("%6d %20.10f\n", n,
                 mc_kinetic_energy(ds) * (temper_factor * temper_factor - 1));
          printf("%6d %20.10f\n", n + 1,
                 mc_kinetic_energy(ds) * (temper_factor * temper_factor - 1));
          if (2 * (n + 1) >= jmps)
            printf("\n");
        }
        // printf("ds-dim= %d",ds->dim);
        for (k = 0; k < ds->dim; k++)
        {
          printf("hybryd visihislenie 3 \n");
          ds->p[k] *= temper_factor;
        }
      }
    }

    /* Make sure we know energy, if needed. */

    if (n < window || n > jmps - window)
    {
      /*printf ("While in hybrid1 %d 5 \n",i);*/
      if (!ds->know_pot)
      {
        // printf ("hybryd visihislenie 4 \n");
        printf("mc_app_energy 1 \n");
        mc_app_energy(ds, 1, 1, &ds->pot_energy, 0);
        ds->know_pot = 1;
      }

      if (!ds->know_kinetic)
      {
        printf("hybryd visihislenie 5 \n");
        ds->kinetic_energy = mc_kinetic_energy(ds);
        ds->know_kinetic = 1;
      }

      //        getchar();
      // 		printf("ds-pot \n");
      H = ds->pot_energy; // + ds->kinetic_energy;
                          //	getchar();
      /* printf ("hybryd visihislenie 5 zaverhenno	 \n"); */
    }
    /*printf ("hybryd visihislenie 12 \n");  */
    /* Account for state in reject window.  Reject window can be ignored if
       windows consist of the entire trajectory. */

    if (window != jmps + 1 && n < window)
    {
      printf("While in hybrid1 %d 6 \n", i);
      rej_free_energy = !have_rej ? H : -addlogs(-rej_free_energy, -H);
      printf("rej_free_energy = %f H= %f \n", rej_free_energy, H);
      //	getchar();

      if (!have_rej || rand_uniform() < exp(rej_free_energy - H))
      {
        printf("hybryd visihislenie 7 \n");
        mc_value_copy(q_rsv, ds->q, ds->dim);
        mc_value_copy(p_rsv, ds->p, ds->dim);
        rej_pot_energy = ds->pot_energy;
        rej_kinetic_energy = ds->kinetic_energy;

        rej_point = n;
        have_rej = 1;
        /*   printf ("hybryd visihislenie 8 \n"); */
      }
    }

    /* Account for state in the accept window. */

    if (n > jmps - window)
    {
      /*printf ("While in hybrid1 %d  6\n",i); */
      printf("hybryd visihislenie 14 \n");
      acc_free_energy = !have_acc ? H : -addlogs(-acc_free_energy, -H);
      printf("acc_free_energy = %f H = %f \n", acc_free_energy, H);
      // getchar();
      if (!have_acc || rand_uniform() < exp(acc_free_energy - H))
      {

        if (n != jmps)
        {
          /* printf ("hybryd visihislenie 9 \n" );*/
          mc_value_copy(q_asv, ds->q, ds->dim);
          mc_value_copy(p_asv, ds->p, ds->dim);
        }

        acc_pot_energy = ds->pot_energy;
        acc_kinetic_energy = ds->kinetic_energy;

        acc_point = n;
        have_acc = 1;
        /* printf ("hybryd visihislenie 10 \n" );*/
      }
    }
  }

  /* Take new state from the appropriate window. */

  if (!have_acc || !have_rej)
    abort();

  it->proposals += 1;
  it->delta = acc_free_energy - rej_free_energy;

  U = rand_uniform(); /* Do every time to keep in sync for coupling purposes */

  double delta = it->delta / it->temperature;
  printf("delta = %f U = %f temperature =  %f \n", exp(-delta), U, it->temperature);
  //	getchar();
  FILE *fp1;
  // fp1 = fopen ("bnn_log1.txt", "a+");
  // fprintf(fp1, "Error =  %f \n",acc_free_energy);
  // fclose(fp1);
  if (it->delta <= 0 || U < exp(-it->delta / it->temperature))
  {
    /*printf ("While in hybrid1 %d  7\n", i); */
    it->move_point = jump * (acc_point - it->window_offset);

    if (acc_point != jmps)
    {
      mc_value_copy(ds->q, q_asv, ds->dim);
      mc_value_copy(ds->p, p_asv, ds->dim);
      ds->pot_energy = acc_pot_energy;
      ds->kinetic_energy = acc_kinetic_energy;
      ds->know_pot = 1;
      ds->know_kinetic = 1;
      ds->know_grad = 0;
    }

    for (k = 0; k < ds->dim; k++)
    {
      ds->p[k] = -ds->p[k];
    }
  }
  else
  {
    /*printf ("While in hybrid1 %d 8 \n",i);*/
    it->rejects += 1;
    it->move_point = jump * (rej_point - it->window_offset);

    mc_value_copy(ds->q, q_rsv, ds->dim);
    mc_value_copy(ds->p, p_rsv, ds->dim);

    ds->pot_energy = rej_pot_energy;
    ds->kinetic_energy = rej_kinetic_energy;
    ds->know_pot = 1;
    ds->know_kinetic = 1;
    ds->know_grad = 0;
  }

  fp = fopen(ds->netFileName, "a+");
  fprintf(fp, "%s \n", "StopHybryb");

  fclose(fp);

  // net_values a =  train_values[6];
  // double *res = chk_alloc (1, sizeof (double));
  // mc_error_calculate(res,ds, 1, 1, &ds->pot_energy, ds->grad);
  // printf ("\n !!!!!!!!!!!! \n _))))))))))))) hybryd end error = %f \n !!!!!!!!!!!! \n",res[0]);
  //  getchar();
  // free(res);
}

/* PERFORM HYBRID MONTE CARLO OPERATION - SECOND FORM. */

void mc_hybrid2(mc_dynamic_state *ds, /* State to update */
                mc_iter *it,          /* Description of this iteration */
                mc_traj *tj,          /* Trajectory specification */
                int steps,            /* Maximum number of steps to do */
                int in_steps,         /* Maximum number of acceptable steps to do */
                int jump,             /* Number of steps in each jump */
                mc_value *q_save,     /* Place to save original q */
                mc_value *p_save      /* Place to save original p */
)
{
  double old_pot_energy, old_kinetic_energy;
  double threshold, H;
  int n, in, k;

  if (steps % jump != 0 || in_steps % jump != 0)
    abort();

  if (!ds->know_pot)
  {
    mc_app_energy(ds, 1, 1, &ds->pot_energy, ds->grad);
    ds->know_pot = 1;
    ds->know_grad = 1;
  }

  if (!ds->know_kinetic)
  {
    ds->kinetic_energy = mc_kinetic_energy(ds);
    ds->know_kinetic = 1;
  }

  mc_value_copy(q_save, ds->q, ds->dim);
  mc_value_copy(p_save, ds->p, ds->dim);

  old_pot_energy = ds->pot_energy;
  old_kinetic_energy = ds->kinetic_energy;

  threshold = ds->pot_energy + ds->kinetic_energy + rand_exp();

  mc_traj_init(tj, it);
  mc_traj_permute();

  n = 0;
  in = 0;

  while (n < steps && in < in_steps)
  {
    mc_trajectory(ds, jump, 1);

    if (!ds->know_pot)
    {
      mc_app_energy(ds, 1, 1, &ds->pot_energy, ds->grad);
      ds->know_pot = 1;
      ds->know_grad = 1;
    }

    if (!ds->know_kinetic)
    {
      ds->kinetic_energy = mc_kinetic_energy(ds);
      ds->know_kinetic = 1;
    }

    H = ds->pot_energy + ds->kinetic_energy;

    n += jump;
    if (H <= threshold)
      in += 1;
  }

  it->proposals += 1;

  if (H <= threshold)
  {
    it->move_point = n;

    for (k = 0; k < ds->dim; k++)
    {
      ds->p[k] = -ds->p[k];
    }
  }
  else
  {
    it->rejects += 1;
    it->move_point = 0;

    mc_value_copy(ds->q, q_save, ds->dim);
    mc_value_copy(ds->p, p_save, ds->dim);

    ds->pot_energy = old_pot_energy;
    ds->kinetic_energy = old_kinetic_energy;
    ds->know_pot = 1;
    ds->know_kinetic = 1;
    ds->know_grad = 0;
  }
}

/* PERFORM SPIRAL DYNAMICS OPERATION. */

#define Spiral_debug 0

void mc_spiral(mc_dynamic_state *ds, /* State to update */
               mc_iter *it,          /* Description of this iteration */
               mc_traj *tj,          /* Trajectory specification */
               int steps,            /* Total number of steps to do */
               double temper_factor, /* Temper factor */
               int dbl,              /* Use a double spiral? */
               mc_value *q_save,     /* Place to save original q */
               mc_value *p_save,     /* Place to save original p */
               mc_value *q_asv,      /* Place to save q values for accept state */
               mc_value *p_asv       /* Place to save p values for accept state */
)
{
  double acc_pot_energy, acc_kinetic_energy, acc_free_energy;
  double old_pot_energy, old_kinetic_energy;
  int offset, switch_point, acc_point;
  int n, k, dir;
  double A, sqtf, lgf;

  sqtf = sqrt(temper_factor);
  lgf = log(temper_factor);

  /* Choose the offset of the start state, and the point to switch to
     a contracting spiral. */

  offset = rand_int(steps + 1);
  switch_point = dbl ? rand_int(steps + 1) : steps;

  /* Save the start state, also make it the accept state at the beginning. */

  mc_value_copy(q_save, ds->q, ds->dim);
  mc_value_copy(q_asv, ds->q, ds->dim);

  mc_value_copy(p_save, ds->p, ds->dim);
  mc_value_copy(p_asv, ds->p, ds->dim);

  if (!ds->know_pot)
  {
    mc_app_energy(ds, 1, 1, &ds->pot_energy, 0);
    ds->know_pot = 1;
  }

  if (!ds->know_kinetic)
  {
    ds->kinetic_energy = mc_kinetic_energy(ds);
    ds->know_kinetic = 1;
  }

  old_pot_energy = ds->pot_energy;
  old_kinetic_energy = ds->kinetic_energy;

  acc_pot_energy = ds->pot_energy;
  acc_kinetic_energy = ds->kinetic_energy;

  acc_free_energy = ds->pot_energy + ds->kinetic_energy + fabs(offset - switch_point) * lgf * ds->dim;

  acc_point = offset;

  /* Set up for travelling about trajectory. */

  mc_traj_init(tj, it);
  mc_traj_permute();

  n = offset;
  dir = -1;

  if (Spiral_debug)
  {
    printf(" n dir q p E K F T pr\n");
    printf("%4d %+d %+10.3e %+6.3f %+6.2f %+6.2f %+6.2f %+6.2f %.3f\n",
           n, dir, ds->q[0], ds->p[0],
           ds->pot_energy, ds->kinetic_energy, fabs(n - switch_point) * lgf * ds->dim,
           ds->pot_energy + ds->kinetic_energy + fabs(n - switch_point) * lgf * ds->dim,
           1.0);
  }

  /* Loop that looks at one new state each iteration. */

  for (;;)
  {
    /* Restore if next state should be a step from the original start state. */

    if (dir == -1 && n == 0)
    {
      mc_value_copy(ds->q, q_save, ds->dim);
      mc_value_copy(ds->p, p_save, ds->dim);

      ds->pot_energy = old_pot_energy;
      ds->kinetic_energy = old_kinetic_energy;

      ds->know_pot = 1;
      ds->know_kinetic = 1;
      ds->know_grad = 0;

      n = offset;
      dir = 1;
    }

    /* See if we're done. */

    if (dir == 1 && n == steps)
      break;

    /* Multiply/divide, do trajectory step, and multiply/divide again. */

    if (dir == 1 ? n >= switch_point : n < switch_point)
    {
      for (k = 0; k < ds->dim; k++)
      {
        ds->p[k] /= sqtf;
      }
    }
    else
    {
      for (k = 0; k < ds->dim; k++)
      {
        ds->p[k] *= sqtf;
      }
    }

    mc_trajectory(ds, dir, 1);

    if (dir == 1 ? n >= switch_point : n < switch_point)
    {
      for (k = 0; k < ds->dim; k++)
      {
        ds->p[k] /= sqtf;
      }
    }
    else
    {
      for (k = 0; k < ds->dim; k++)
      {
        ds->p[k] *= sqtf;
      }
    }

    /* Make sure we know energy. */

    if (!ds->know_pot)
    {
      mc_app_energy(ds, 1, 1, &ds->pot_energy, 0);
      ds->know_pot = 1;
    }

    ds->kinetic_energy = mc_kinetic_energy(ds);
    ds->know_kinetic = 1;

    /* Advance to next position. */

    n += dir;

    /* Update accepted point and accept free energy accounting for this state.*/

    A = ds->pot_energy + ds->kinetic_energy + fabs(n - switch_point) * lgf * ds->dim;

    acc_free_energy = -addlogs(-acc_free_energy, -A);

    if (Spiral_debug)
    {
      printf("%4d %+d %+10.3e %+6.3f %+6.2f %+6.2f %+6.2f %+6.2f %.3f\n",
             n, dir, ds->q[0], ds->p[0],
             ds->pot_energy, ds->kinetic_energy, fabs(n - switch_point) * lgf * ds->dim,
             A, exp(acc_free_energy - A));
    }

    if (rand_uniform() < exp(acc_free_energy - A))
    {
      mc_value_copy(q_asv, ds->q, ds->dim);
      mc_value_copy(p_asv, ds->p, ds->dim);

      acc_pot_energy = ds->pot_energy;
      acc_kinetic_energy = ds->kinetic_energy;

      acc_point = n;
    }
  }

  /* Go to the point that was accepted in the end. */

  it->move_point = acc_point - offset;
  it->spiral_offset = offset;
  it->spiral_switch = switch_point;

  mc_value_copy(ds->q, q_asv, ds->dim);
  mc_value_copy(ds->p, p_asv, ds->dim);

  ds->pot_energy = acc_pot_energy;
  ds->kinetic_energy = acc_kinetic_energy;
  ds->know_pot = 1;
  ds->know_kinetic = 1;
  ds->know_grad = 0;
}
