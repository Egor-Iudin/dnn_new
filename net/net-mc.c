/* NET-MC.C - Interface between neural network and Markov chain modules. */

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

/* Some functions were modified to account for the training cases' weights:
 *  mc_app_initialize: in what concerns train_sumsq calculation,
 *  gibbs_noise & rgrid_met_noise: in what concerns calculation of the
 * difference between the outputs and targets (strored in variable d),
 *  mc_app_energy: in what concerns calculation of the log-prob and its gradient
 * -- Andrey Popov
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "misc.h"
#include "rand.h"
#include "log.h"
#include "mc.h"
#include "data.h"
#include "prior.h"
#include "model.h"
#include "net.h"
#include "net-data.h"

/* SHOULD A CHEAP ENERGY FUNCTION BE USED?  If set to 0, the full energy
   function is used, equal to minus the log of the probability of the
   training data, given the current weights and noise hyperparameters.
   This is necessary if marginal likelihoods are to be found using Annealed
   Importance Sampling.  If set to 1, the energy omits constant terms.
   If set to 2, the energy omits terms involving the noise hyperparameters,
   which is OK for sampling weights with hybrid Monte Carlo, etc., but does
   not work when tempering or annealing schemes are used. */

#define Cheap_energy 0 /* Normally set to 0 */

/* NETWORK VARIABLES. */

static int initialize_done = 0; /* Has this all been set up? */

static net_arch *arch;             /* Network architecture */
static net_flags *flgs;            /* Network flags, null if none */
static model_specification *model; /* Data model */
static net_priors *priors;         /* Network priors */
static model_survival *surv;       /* Hazard type for survival model */

static net_sigmas sigmas; /* Hyperparameters for network, auxiliary state
           for Monte Carlo */

static net_params params; /* Pointers to parameters, which are position
           coordinates for dynamical Monte Carlo */

static net_params stepsizes; /* Pointers to stepsizes */
static net_values seconds;   /* Second derivatives */
static double *train_sumsq;  /* Sums of squared training input values */

static net_values *deriv; /* Derivatives for training cases */
static net_params grad;   /* Pointers to gradient for network parameters*/

static double *quadratic_approx; /* Quadratic approximation to log likelihood */

/* PROCEDURES. */

static void gibbs_noise(int, double);

static void gibbs_unit(int, net_param *, net_sigma *, net_sigma *,
                       int, prior_spec *);

static void gibbs_conn(int, net_param *, net_sigma *, net_sigma *, net_sigma *,
                       int, int, prior_spec *);

static void gibbs_adjustments(net_sigma *, double, int,
                              net_param *, net_sigma *, double,
                              net_param *, net_sigma *, double, int,
                              int, int *, net_param **, net_sigma **,
                              prior_spec *, int *);

static void rgrid_met_noise(double, mc_iter *, double);

static void rgrid_met_unit(double, mc_iter *,
                           net_param *, net_sigma *, net_sigma *,
                           int, prior_spec *);

static void rgrid_met_conn(double, mc_iter *,
                           net_param *, net_sigma *, net_sigma *, net_sigma *,
                           int, int, prior_spec *);

static double sum_squares(net_param *, net_sigma *, int);

static double rgrid_sigma(double, mc_iter *, double,
                          double, double, double, double, int);

/* SET UP REQUIRED RECORD SIZES PRIOR TO GOBBLING RECORDS. */

void mc_app_record_sizes(log_gobbled *logg /* Structure to hold gobbled data */
)
{
  net_record_sizes(logg);
}

/* INITIALIZE AND SET UP DYNAMIC STATE STRUCTURE.  Skips some stuff
   if it's already been done, as indicated by the initialize_done
   variable. */

void mc_app_initialize(log_gobbled *logg,   /* Records gobbled up from head and tail of log file */
                       mc_dynamic_state *ds /* Structure holding pointers to dynamical state */
)
{
  net_value *value_block;
  int value_count;
  int i, j;
  printf("\n mc_app_initialize \n");
  // getchar();
  if (!initialize_done)
  {
    /* Check that required specification records are present. */
    // printf("First Iteration doing \n");
    // getchar();
    arch = logg->data['A'];
    flgs = logg->data['F'];
    model = logg->data['M'];
    priors = logg->data['P'];
    surv = logg->data['V'];

    net_check_specs_present(arch, priors, 0, model, surv);

    if (model != 0 && model->type == 'R' && model->autocorr)
    {
      fprintf(stderr, "Can't handle autocorrelated noise in net-mc\n");
      exit(1);
    }

    /* Look for quadratic approximation record.  If there is one, we use it. */

    quadratic_approx = logg->data['Q'];

    /* Locate existing network, if one exists. */

    sigmas.total_sigmas = net_setup_sigma_count(arch, flgs, model);
    params.total_params = net_setup_param_count(arch, flgs);

    sigmas.sigma_block = logg->data['S'];
    params.param_block = logg->data['W'];

    grad.total_params = params.total_params;

    if (sigmas.sigma_block != 0 || params.param_block != 0)
    {
      printf("Create new Network 3\n");
      if (sigmas.sigma_block == 0 || logg->index['S'] != logg->last_index || params.param_block == 0 || logg->index['W'] != logg->last_index)
      {
        fprintf(stderr,
                "Network stored in log file is apparently incomplete\n");
        exit(1);
      }

      if (logg->actual_size['S'] != sigmas.total_sigmas * sizeof(net_sigma) || logg->actual_size['W'] != params.total_params * sizeof(net_param))
      {
        fprintf(stderr, "Bad size for network record\n");
        exit(1);
      }

      net_setup_sigma_pointers(&sigmas, arch, flgs, model);
      net_setup_param_pointers(&params, arch, flgs);
    }
    else
    {
      // printf("Create new Network 4\n");

      // getchar();
      sigmas.sigma_block = chk_alloc(sigmas.total_sigmas, sizeof(net_sigma));
      params.param_block = chk_alloc(params.total_params, sizeof(net_param));

      net_setup_sigma_pointers(&sigmas, arch, flgs, model);
      net_setup_param_pointers(&params, arch, flgs);

      net_prior_generate(&params, &sigmas, arch, flgs, model, priors, 1, 0, 0);
    }

    /* Set up stepsize structure. */

    stepsizes.total_params = params.total_params;
    stepsizes.param_block = chk_alloc(params.total_params, sizeof(net_param));

    net_setup_param_pointers(&stepsizes, arch, flgs);

    /* Set up second derivative structure. */

    value_count = net_setup_value_count(arch);
    value_block = chk_alloc(value_count, sizeof *value_block);

    net_setup_value_pointers(&seconds, value_block, arch);

    /* Read training data, if any, and allocate space for derivatives. */

    data_spec = logg->data['D'];

    if (data_spec != 0 && model == 0)
    {
      fprintf(stderr, "No model specified for data\n");
      exit(1);
    }

    if (data_spec && logg->actual_size['D'] !=
                         data_spec_size(data_spec->N_inputs, data_spec->N_targets))
    {
      fprintf(stderr, "Data specification record is the wrong size!\n");
      exit(1);
    }

    train_sumsq = chk_alloc(arch->N_inputs, sizeof *train_sumsq);
    for (j = 0; j < arch->N_inputs; j++)
      train_sumsq[j] = 0;

    if (data_spec != 0)
    {
      // printf("Create new Network  1\n");
      // getchar();
      net_data_read(1, 1, arch, model, surv); // Reading the training data

      deriv = chk_alloc(N_train, sizeof *deriv);

      value_count = net_setup_value_count(arch);
      value_block = chk_alloc(value_count * N_train, sizeof *value_block);

      for (i = 0; i < N_train; i++)
      {
        net_setup_value_pointers(&deriv[i], value_block + value_count * i, arch);
      }

      for (j = 0; j < arch->N_inputs; j++)
      {
        for (i = 0; i < N_train; i++)
        { // Modified to make use of the training cases' weights
          if (data_spec->has_weights)
            train_sumsq[j] += train_values[i].i[j] * train_values[i].i[j] *
                              train_weights[i];
          else
            train_sumsq[j] += train_values[i].i[j] * train_values[i].i[j];
        }
      }

      if (model != 0 && model->type == 'V' && surv->hazard_type != 'C')
      {
        // printf("Create new Network 2\n");
        // getchar();
        double tsq;
        int n;

        tsq = 0;

        for (n = 0; surv->time[n] != 0; n++)
        {
          if (n == Max_time_points)
            abort();
          tsq += surv->log_time ? log(surv->time[n]) * log(surv->time[n])
                                : surv->time[n] * surv->time[n];
        }

        train_sumsq[0] = N_train * tsq / n;
      }
    }

    /* Make sure we don't do all this again. */

    initialize_done = 1;
  }

  /* Set up Monte Carlo state structure. */

  ds->aux_dim = sigmas.total_sigmas;
  ds->aux = sigmas.sigma_block;

  ds->dim = params.total_params;

  // printf("AAAAAAAAAAAAAAAAAAAAAA \n");
  ds->q = params.param_block;
  // for (k = 0; k<ds->dim; k++)
  //         {
  //            ds->q[k] ;
  //        }
  ds->temp_state = 0;

  ds->stepsize = stepsizes.param_block;

  if (quadratic_approx && logg->actual_size['Q'] != (1 + ds->dim + ds->dim * ds->dim) * sizeof *quadratic_approx)
  {
    fprintf(stderr, "Approximation record is the wrong size (%d!=%d)\n",
            logg->actual_size['Q'],
            (1 + ds->dim + ds->dim * ds->dim) * sizeof *quadratic_approx);
    exit(1);
  }
}

/* RESET INITIALIZE_DONE IN PREPARATION FOR NEW LOG FILE. */

void net_mc_cleanup(void)
{
  initialize_done = 0;
}

/* SAVE POSITION AND AUXILIARY PART OF STATE. */

void mc_app_save(mc_dynamic_state *ds, /* Current dyanamical state */
                 log_file *logf,       /* Log file state structure */
                 int index             /* Index of iteration being saved */
)
{
  // printf("BBBBBBBBBb\n");
  logf->header.type = 'S';
  logf->header.index = index;
  logf->header.size = sigmas.total_sigmas * sizeof(net_sigma);
  log_file_append(logf, sigmas.sigma_block);

  logf->header.type = 'W';
  logf->header.index = index;
  logf->header.size = params.total_params * sizeof(net_param);
  log_file_append(logf, params.param_block);
}

/* APPLICATION-SPECIFIC SAMPLING PROCEDURE.  Does gibbs sampling for
   hyperparameters ("sample-hyper"), or for noise levels ("sample-noise"),
   or for both ("sample-sigmas"), or does things separately for "upper"
   or "lower" level hyperparameters. */

int mc_app_sample(mc_dynamic_state *ds,
                  char *op,
                  double pm,
                  double pm2,
                  mc_iter *it,
                  mc_temp_sched *sch)
{
  int sample_hyper, sample_noise, rgrid_hyper, rgrid_noise;
  int l;

  if (pm == 0)
    pm = 0.1;

  sample_hyper = sample_noise = rgrid_hyper = rgrid_noise = 0;

  if (strcmp(op, "sample-sigmas") == 0)
  {
    sample_hyper = sample_noise = 1;
  }
  else if (strcmp(op, "sample-hyper") == 0)
  {
    sample_hyper = 1;
  }
  else if (strcmp(op, "sample-noise") == 0)
  {
    sample_noise = 1;
  }
  else if (strcmp(op, "sample-lower-sigmas") == 0)
  {
    sample_hyper = sample_noise = -1;
  }
  else if (strcmp(op, "sample-lower-hyper") == 0)
  {
    sample_hyper = -1;
  }
  else if (strcmp(op, "sample-lower-noise") == 0)
  {
    sample_noise = -1;
  }
  else if (strcmp(op, "rgrid-upper-sigmas") == 0)
  {
    rgrid_hyper = rgrid_noise = 1;
  }
  else if (strcmp(op, "rgrid-upper-hyper") == 0)
  {
    rgrid_hyper = 1;
  }
  else if (strcmp(op, "rgrid-upper-noise") == 0)
  {
    rgrid_noise = 1;
  }
  else
  {
    return 0;
  }

  if (rgrid_noise && model->type == 'R')
  {
    rgrid_met_noise(pm, it, !ds->temp_state ? 1 : ds->temp_state->inv_temp);
  }

  if (rgrid_hyper)
  {
    if (arch->has_ti)
      rgrid_met_unit(pm, it,
                     params.ti, sigmas.ti_cm, 0,
                     arch->N_inputs, &priors->ti);

    for (l = 0; l < arch->N_layers; l++)
    {
      // printf("Bom Bom %d \n",l);
      if (l > 0)
      {
        if (arch->has_hh[l - 1])
        {
          rgrid_met_conn(pm, it,
                         params.hh[l - 1], sigmas.hh_cm[l - 1], sigmas.hh[l - 1],
                         sigmas.ah[l], arch->N_hidden[l - 1], arch->N_hidden[l],
                         &priors->hh[l - 1]);
        }
      }

      if (arch->has_ih[l])
      {
        rgrid_met_conn(pm, it,
                       params.ih[l], sigmas.ih_cm[l], sigmas.ih[l], sigmas.ah[l],
                       not_omitted(flgs ? flgs->omit : 0, arch->N_inputs, 1 << (l + 1)),
                       arch->N_hidden[l], &priors->ih[l]);
      }

      if (arch->has_bh[l])
        rgrid_met_unit(pm, it,
                       params.bh[l], sigmas.bh_cm[l],
                       sigmas.ah[l], arch->N_hidden[l],
                       &priors->bh[l]);

      if (arch->has_th[l])
        rgrid_met_unit(pm, it,
                       params.th[l], sigmas.th_cm[l], 0,
                       arch->N_hidden[l], &priors->th[l]);

      if (arch->has_ho[l])
      {
        rgrid_met_conn(pm, it,
                       params.ho[l], sigmas.ho_cm[l], sigmas.ho[l], sigmas.ao,
                       arch->N_hidden[l], arch->N_outputs, &priors->ho[l]);
      }
    }

    if (arch->has_io)
    {
      rgrid_met_conn(pm, it,
                     params.io, sigmas.io_cm, sigmas.io, sigmas.ao,
                     not_omitted(flgs ? flgs->omit : 0, arch->N_inputs, 1),
                     arch->N_outputs, &priors->io);
    }

    if (arch->has_bo)
      rgrid_met_unit(pm, it,
                     params.bo, sigmas.bo_cm, sigmas.ao,
                     arch->N_outputs, &priors->bo);
  }

  if (sample_noise && model->type == 'R')
  {
    gibbs_noise(sample_noise, !ds->temp_state ? 1 : ds->temp_state->inv_temp);
  }

  if (sample_hyper)
  {
    if (arch->has_ti)
      gibbs_unit(sample_hyper, params.ti, sigmas.ti_cm, 0,
                 arch->N_inputs, &priors->ti);

    for (l = 0; l < arch->N_layers; l++)
    {
      /*  printf("Bom1 Bom1 %d \n",l); */
      if (l > 0)
      {
        if (arch->has_hh[l - 1])
        {
          gibbs_conn(sample_hyper,
                     params.hh[l - 1], sigmas.hh_cm[l - 1], sigmas.hh[l - 1],
                     sigmas.ah[l], arch->N_hidden[l - 1], arch->N_hidden[l],
                     &priors->hh[l - 1]);
        }
      }

      if (arch->has_ih[l])
      {
        gibbs_conn(sample_hyper,
                   params.ih[l], sigmas.ih_cm[l], sigmas.ih[l], sigmas.ah[l],
                   not_omitted(flgs ? flgs->omit : 0, arch->N_inputs, 1 << (l + 1)),
                   arch->N_hidden[l], &priors->ih[l]);
      }

      if (arch->has_bh[l])
        gibbs_unit(sample_hyper,
                   params.bh[l], sigmas.bh_cm[l],
                   sigmas.ah[l], arch->N_hidden[l],
                   &priors->bh[l]);

      if (arch->has_th[l])
        gibbs_unit(sample_hyper,
                   params.th[l], sigmas.th_cm[l], 0,
                   arch->N_hidden[l], &priors->th[l]);

      if (arch->has_ah[l])
      {
        gibbs_adjustments(sigmas.ah[l], priors->ah[l], arch->N_hidden[l],
                          arch->has_bh[l] ? params.bh[l] : 0, sigmas.bh_cm[l],
                          priors->bh[l].alpha[1],
                          arch->has_ih[l] ? params.ih[l] : 0, sigmas.ih[l],
                          priors->ih[l].alpha[2],
                          not_omitted(flgs ? flgs->omit : 0, arch->N_inputs, 1 << (l + 1)),
                          l > 0, &arch->has_hh[l - 1], &params.hh[l - 1], &sigmas.hh[l - 1],
                          &priors->hh[l - 1], &arch->N_hidden[l - 1]);
      }

      if (arch->has_ho[l])
      {
        gibbs_conn(sample_hyper,
                   params.ho[l], sigmas.ho_cm[l], sigmas.ho[l], sigmas.ao,
                   arch->N_hidden[l], arch->N_outputs, &priors->ho[l]);
      }
    }
    /*  printf("Bom1 Bom1 End %d \n",l);*/
    if (arch->has_io)
    {
      gibbs_conn(sample_hyper,
                 params.io, sigmas.io_cm, sigmas.io, sigmas.ao,
                 not_omitted(flgs ? flgs->omit : 0, arch->N_inputs, 1),
                 arch->N_outputs, &priors->io);
    }

    if (arch->has_bo)
      gibbs_unit(sample_hyper,
                 params.bo, sigmas.bo_cm, sigmas.ao,
                 arch->N_outputs, &priors->bo);

    if (arch->has_ao)
    {
      gibbs_adjustments(sigmas.ao, priors->ao, arch->N_outputs,
                        arch->has_bo ? params.bo : 0,
                        sigmas.bo_cm,
                        priors->bo.alpha[1],
                        arch->has_io ? params.io : 0,
                        sigmas.io,
                        priors->io.alpha[2],
                        not_omitted(flgs ? flgs->omit : 0, arch->N_inputs, 1),
                        arch->N_layers, arch->has_ho, params.ho, sigmas.ho,
                        priors->ho, arch->N_hidden);
    }
  }

  ds->know_pot = 0;
  ds->know_grad = 0;

  return 1;
}

/* DO GIBBS SAMPLING FOR NOISE SIGMAS. */

static void gibbs_noise(int sample_noise, /* +1 for all, -1 for lower only */
                        double inv_temp)
{
  double nalpha, nprec, sum, d, ps;
  prior_spec *pr;
  int i, j;

  for (i = 0; i < N_train; i++)
  {
    //   printf("Bom2 Bom2 %d \n",i);
    net_func(&train_values[i], 0, arch, flgs, &params);
  }
  // printf("Bom2 Bom2 End %d \n",i);
  pr = &model->noise;

  if (pr->alpha[1] != 0 && pr->alpha[2] == 0)
  {
    for (j = 0; j < arch->N_outputs; j++)
    {
      //   printf("Bom4 Bo4 %d \n",j);
      sum = pr->alpha[1] * (*sigmas.noise_cm * *sigmas.noise_cm);
      for (i = 0; i < N_train; i++)
      {
        d = train_values[i].o[j] - train_targets[i * arch->N_outputs + j];

        // Modified to make use of the training cases' weights
        if (data_spec->has_weights)
          sum += inv_temp * d * d * train_weights[i];
        else
          sum += inv_temp * d * d;
      }

      nalpha = pr->alpha[1] + inv_temp * N_train;
      nprec = nalpha / sum;

      sigmas.noise[j] = prior_pick_sigma(1 / sqrt(nprec), nalpha);
    }
  }

  if (pr->alpha[1] != 0 && pr->alpha[2] != 0 && sample_noise > 0)
  {
    for (j = 0; j < arch->N_outputs; j++)
    {
      // printf("Bom4 Bom4 End %d \n",j);
      //  printf("Bom5 Bom5 %d \n",j);
      ps = pr->alpha[2] * (sigmas.noise[j] * sigmas.noise[j]);

      sum = 0;
      for (i = 0; i < N_train; i++)
      {
        d = train_values[i].o[j] - train_targets[i * arch->N_outputs + j];

        // Modified to make use of the training cases' weights
        if (data_spec->has_weights)
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2) *
                 train_weights[i];
        else
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2);
      }

      sigmas.noise[j] = cond_sigma(*sigmas.noise_cm, pr->alpha[1],
                                   pr->alpha[2], sum, N_train);
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] == 0 && pr->alpha[2] == 0)
  {
    sum = pr->alpha[0] * (pr->width * pr->width);
    for (i = 0; i < N_train; i++)
    {
      //  printf("Bom6 Bom6 %d \n",i);
      for (j = 0; j < arch->N_outputs; j++)
      {
        d = train_values[i].o[j] - train_targets[i * arch->N_outputs + j];

        // Modified to make use of the training cases' weights
        if (data_spec->has_weights)
          sum += inv_temp * d * d * train_weights[i];
        else
          sum += inv_temp * d * d;
      }
    }
    // printf("Bom1 Bom1 End %d \n",i);
    nalpha = pr->alpha[0] + inv_temp * N_train * arch->N_outputs;
    nprec = nalpha / sum;
    *sigmas.noise_cm = prior_pick_sigma(1 / sqrt(nprec), nalpha);

    for (j = 0; j < arch->N_outputs; j++)
    { //  printf("Bom7 Bom7 %d \n",j);
      sigmas.noise[j] = *sigmas.noise_cm;
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] == 0 && pr->alpha[2] != 0 && sample_noise > 0)
  {
    ps = pr->alpha[2] * (*sigmas.noise_cm * *sigmas.noise_cm);

    sum = 0;
    for (i = 0; i < N_train; i++)
    {
      //  printf("Bom8 Bom8 %d \n",i);
      for (j = 0; j < arch->N_outputs; j++)
      {
        d = train_values[i].o[j] - train_targets[i * arch->N_outputs + j];

        // Modified to make use of the training cases' weights
        if (data_spec->has_weights)
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2) *
                 train_weights[i];
        else
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2);
      }
    }
    // printf("Bom8 Bom8 End %d \n",i);
    *sigmas.noise_cm = cond_sigma(pr->width, pr->alpha[0],
                                  pr->alpha[2], sum, arch->N_outputs * N_train);

    for (j = 0; j < arch->N_outputs; j++)
    {
      sigmas.noise[j] = *sigmas.noise_cm;
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] != 0 && sample_noise > 0)
  {
    sum = 0;
    for (j = 0; j < arch->N_outputs; j++)
    {
      sum += 1 / (sigmas.noise[j] * sigmas.noise[j]);
    }

    *sigmas.noise_cm = cond_sigma(pr->width, pr->alpha[0],
                                  pr->alpha[1], sum, arch->N_outputs);
  }
}

/* DO RANDOM-GRID METROPOLIS UPDATES FOR UPPER NOISE SIGMAS. */

static void rgrid_met_noise(double stepsize, /* Stepsize for update */
                            mc_iter *it,
                            double inv_temp)
{
  double nalpha, nprec, sum, d, ps;
  prior_spec *pr;
  int i, j;

  for (i = 0; i < N_train; i++)
  {
    //   printf("Bom9 Bom9 %d \n",i);
    net_func(&train_values[i], 0, arch, flgs, &params);
  }

  pr = &model->noise;

  if (pr->alpha[1] != 0 && pr->alpha[2] != 0)
  {
    for (j = 0; j < arch->N_outputs; j++)
    {
      ps = pr->alpha[2] * (sigmas.noise[j] * sigmas.noise[j]);

      sum = 0;
      for (i = 0; i < N_train; i++)
      {
        //  printf("Bom10 Bom10 %d \n",i);
        d = train_values[i].o[j] - train_targets[i * arch->N_outputs + j];

        // Modified to make use of the training cases' weights
        if (data_spec->has_weights)
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2) *
                 train_weights[i];
        else
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2);
      }
      //  printf("Bom10 Bom10 End %d \n",i);
      sigmas.noise[j] = rgrid_sigma(stepsize, it, sigmas.noise[j],
                                    *sigmas.noise_cm, pr->alpha[1],
                                    pr->alpha[2], sum, N_train);
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] == 0 && pr->alpha[2] != 0)
  {
    ps = pr->alpha[2] * (*sigmas.noise_cm * *sigmas.noise_cm);

    sum = 0;
    for (i = 0; i < N_train; i++)
    {
      for (j = 0; j < arch->N_outputs; j++)
      {
        d = train_values[i].o[j] - train_targets[i * arch->N_outputs + j];

        // Modified to make use of the training cases' weights
        if (data_spec->has_weights)
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2) *
                 train_weights[i];
        else
          sum += rand_gamma((pr->alpha[2] + inv_temp) / 2) / ((ps + inv_temp * d * d) / 2);
      }
    }

    *sigmas.noise_cm = rgrid_sigma(stepsize, it, *sigmas.noise_cm,
                                   pr->width, pr->alpha[0],
                                   pr->alpha[2], sum, arch->N_outputs * N_train);

    for (j = 0; j < arch->N_outputs; j++)
    {
      sigmas.noise[j] = *sigmas.noise_cm;
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] != 0)
  {
    sum = 0;
    for (j = 0; j < arch->N_outputs; j++)
    {
      sum += 1 / (sigmas.noise[j] * sigmas.noise[j]);
    }

    *sigmas.noise_cm = rgrid_sigma(stepsize, it, *sigmas.noise_cm,
                                   pr->width, pr->alpha[0],
                                   pr->alpha[1], sum, arch->N_outputs);
  }
}

/* DO GIBBS SAMPLING FOR SIGMA ASSOCIATED WITH GROUP OF UNITS. */

static void gibbs_unit(int sample_hyper, /* +1 for all, -1 for lower only */
                       net_param *wt,    /* Parameters associated with each unit */
                       net_sigma *sg_cm, /* Common sigma controlling parameter distribution */
                       net_sigma *adj,   /* Adjustments for each unit, or zero */
                       int n,            /* Number of units */
                       prior_spec *pr    /* Prior for sigmas */
)
{
  double nalpha, nprec, sum, ps, d;
  int i;

  if (pr->alpha[0] != 0 && pr->alpha[1] == 0)
  {
    nalpha = pr->alpha[0] + n;

    nprec = nalpha / (pr->alpha[0] * (pr->width * pr->width) + sum_squares(wt, adj, n));

    *sg_cm = prior_pick_sigma(1 / sqrt(nprec), nalpha);
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] != 0 && sample_hyper > 0)
  {
    ps = pr->alpha[1] * (*sg_cm * *sg_cm);

    sum = 0;
    for (i = 0; i < n; i++)
    {
      d = adj == 0 ? wt[i] : wt[i] / adj[i];
      sum += rand_gamma((pr->alpha[1] + 1) / 2) / ((ps + d * d) / 2);
    }

    *sg_cm = cond_sigma(pr->width, pr->alpha[0], pr->alpha[1], sum, n);
  }
}

/* DO RANDOM-GRID UPDATES FOR UPPER SIGMA ASSOCIATED WITH GROUP OF UNITS. */

static void rgrid_met_unit(double stepsize, /* Stepsize for update */
                           mc_iter *it,
                           net_param *wt,    /* Parameters associated with each unit */
                           net_sigma *sg_cm, /* Common sigma controlling parameter distribution */
                           net_sigma *adj,   /* Adjustments for each unit, or zero */
                           int n,            /* Number of units */
                           prior_spec *pr    /* Prior for sigmas */
)
{
  double nalpha, nprec, sum, ps, d;
  int i;

  if (pr->alpha[0] != 0 && pr->alpha[1] != 0)
  {
    ps = pr->alpha[1] * (*sg_cm * *sg_cm);

    sum = 0;
    for (i = 0; i < n; i++)
    {
      d = adj == 0 ? wt[i] : wt[i] / adj[i];
      sum += rand_gamma((pr->alpha[1] + 1) / 2) / ((ps + d * d) / 2);
    }

    *sg_cm = rgrid_sigma(stepsize, it, *sg_cm,
                         pr->width, pr->alpha[0], pr->alpha[1], sum, n);
  }
}

/* DO GIBBS SAMPLING FOR SIGMAS ASSOCIATED WITH GROUP OF CONNECTIONS. */

static void gibbs_conn(int sample_hyper, /* +1 for all, -1 for lower only */
                       net_param *wt,    /* Weights on connections */
                       net_sigma *sg_cm, /* Common sigma controlling weights */
                       net_sigma *sg,    /* Individual sigmas for source units */
                       net_sigma *adj,   /* Adjustments for each destination unit, or zero */
                       int ns,           /* Number of source units */
                       int nd,           /* Number of destination units */
                       prior_spec *pr    /* Prior for sigmas */
)
{
  double width, nalpha, nprec, sum, ps, d;
  int i, j;

  width = prior_width_scaled(pr, ns);

  if (pr->alpha[1] != 0 && pr->alpha[2] == 0)
  {
    for (i = 0; i < ns; i++)
    {
      nalpha = pr->alpha[1] + nd;
      nprec = nalpha / (pr->alpha[1] * (*sg_cm * *sg_cm) + sum_squares(wt + nd * i, adj, nd));

      sg[i] = prior_pick_sigma(1 / sqrt(nprec), nalpha);
    }
  }

  if (pr->alpha[1] != 0 && pr->alpha[2] != 0 && sample_hyper > 0)
  {
    for (i = 0; i < ns; i++)
    {

      // printf("Bom12 Bom12 %d \n",i);
      ps = pr->alpha[2] * (sg[i] * sg[i]);

      sum = 0;
      for (j = 0; j < nd; j++)
      {
        d = adj == 0 ? wt[nd * i + j] : wt[nd * i + j] / adj[j];
        sum += rand_gamma((pr->alpha[2] + 1) / 2) / ((ps + d * d) / 2);
      }

      sg[i] = cond_sigma(*sg_cm, pr->alpha[1], pr->alpha[2], sum, nd);
    }
    // printf("Bom12 Bom12 End %d \n",i);
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] == 0 && pr->alpha[2] == 0)
  {
    nalpha = pr->alpha[0] + ns * nd;

    sum = pr->alpha[0] * (width * width);
    for (i = 0; i < ns; i++)
    {
      sum += sum_squares(wt + nd * i, adj, nd);
    }
    nprec = nalpha / sum;

    *sg_cm = prior_pick_sigma(1 / sqrt(nprec), nalpha);

    for (i = 0; i < ns; i++)
    {
      sg[i] = *sg_cm;
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] == 0 && pr->alpha[2] != 0 && sample_hyper > 0)
  {
    ps = pr->alpha[2] * (*sg_cm * *sg_cm);

    sum = 0;
    for (i = 0; i < ns; i++)
    {
      for (j = 0; j < nd; j++)
      {
        d = adj == 0 ? wt[nd * i + j] : wt[nd * i + j] / adj[j];
        sum += rand_gamma((pr->alpha[2] + 1) / 2) / ((ps + d * d) / 2);
      }
    }

    *sg_cm = cond_sigma(width, pr->alpha[0], pr->alpha[2], sum, ns * nd);

    for (i = 0; i < ns; i++)
    {
      sg[i] = *sg_cm;
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] != 0 && sample_hyper > 0)
  {
    sum = 0;
    for (i = 0; i < ns; i++)
    {
      sum += 1 / (sg[i] * sg[i]);
    }

    *sg_cm = cond_sigma(width, pr->alpha[0], pr->alpha[1], sum, ns);
  }
}

/* DO RANDOM-GRID UPDATES FOR UPPER SIGMAS ASSOCIATED WITH GROUP OF CONNECTIONS.
 */

static void rgrid_met_conn(double stepsize, /* Stepsize for update */
                           mc_iter *it,
                           net_param *wt,    /* Weights on connections */
                           net_sigma *sg_cm, /* Common sigma controlling weights */
                           net_sigma *sg,    /* Individual sigmas for source units */
                           net_sigma *adj,   /* Adjustments for each destination unit, or zero */
                           int ns,           /* Number of source units */
                           int nd,           /* Number of destination units */
                           prior_spec *pr    /* Prior for sigmas */
)
{
  double width, nalpha, nprec, sum, ps, d;
  int i, j;

  width = prior_width_scaled(pr, ns);

  if (pr->alpha[1] != 0 && pr->alpha[2] != 0)
  {
    for (i = 0; i < ns; i++)
    {
      //   printf("Bom13 Bom13 %d \n",i);
      ps = pr->alpha[2] * (sg[i] * sg[i]);

      sum = 0;
      for (j = 0; j < nd; j++)
      {
        d = adj == 0 ? wt[nd * i + j] : wt[nd * i + j] / adj[j];
        sum += rand_gamma((pr->alpha[2] + 1) / 2) / ((ps + d * d) / 2);
      }

      sg[i] = rgrid_sigma(stepsize, it, sg[i],
                          *sg_cm, pr->alpha[1], pr->alpha[2], sum, nd);
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] == 0 && pr->alpha[2] != 0)
  {
    ps = pr->alpha[2] * (*sg_cm * *sg_cm);

    sum = 0;
    for (i = 0; i < ns; i++)
    {
      for (j = 0; j < nd; j++)
      {
        d = adj == 0 ? wt[nd * i + j] : wt[nd * i + j] / adj[j];
        sum += rand_gamma((pr->alpha[2] + 1) / 2) / ((ps + d * d) / 2);
      }
    }

    *sg_cm = rgrid_sigma(stepsize, it, *sg_cm,
                         width, pr->alpha[0], pr->alpha[2], sum, ns * nd);

    for (i = 0; i < ns; i++)
    {
      sg[i] = *sg_cm;
    }
  }

  if (pr->alpha[0] != 0 && pr->alpha[1] != 0)
  {
    sum = 0;
    for (i = 0; i < ns; i++)
    {
      sum += 1 / (sg[i] * sg[i]);
    }

    *sg_cm = rgrid_sigma(stepsize, it, *sg_cm,
                         width, pr->alpha[0], pr->alpha[1], sum, ns);
  }
}

/* DO GIBBS SAMPLING FOR UNIT ADJUSTMENTS. */

static void gibbs_adjustments(net_sigma *adj, /* Adjustments to sample for */
                              double alpha,   /* Alpha for adjustments */
                              int nd,         /* Number of units with adjustments */
                              net_param *b,   /* Biases for destination units, or zero */
                              net_sigma *s,   /* Sigma associated with biases */
                              double a,       /* Alpha for this sigma */
                              net_param *w1,  /* First set of weights, or zero */
                              net_sigma *s1,  /* Sigmas associated with first set */
                              double a1,      /* Alpha for first set */
                              int n1,         /* Number of source units for first set */
                              int nrem,       /* Number of remaining weight sets */
                              int *has,       /* Whether each remaining set is present */
                              net_param **wr, /* Remaining sets of weights */
                              net_sigma **sr, /* Sigmas associated with remaining sets */
                              prior_spec *ar, /* Priors for remaining sets */
                              int *nr         /* Numbers of source units in remaining sets */
)
{
  double nalpha, nprec, sum, d, ad;
  int r, i, j;

  for (i = 0; i < nd; i++)
  {
    nalpha = alpha;
    sum = alpha;

    ad = adj[i];

    if (b != 0)
    {
      nalpha += 1;

      d = b[i];
      if (a == 0)
      {
        d /= *s;
      }
      else
      {
        d /= prior_pick_sigma(sqrt((a * (*s * *s) + (d * d) / (ad * ad)) / (a + 1)), a + 1);
      }
      sum += d * d;
    }

    if (w1 != 0)
    {
      nalpha += n1;

      for (j = 0; j < n1; j++)
      {
        d = w1[nd * j + i];
        if (a1 == 0)
        {
          d /= s1[j];
        }
        else
        {
          d /= prior_pick_sigma(sqrt((a1 * (s1[j] * s1[j]) + (d * d) / (ad * ad)) / (a1 + 1)), a1 + 1);
        }
        sum += d * d;
      }
    }

    for (r = 0; r < nrem; r++)
    {
      // printf("Bom14 Bom14 %d \n",r);
      if (has[r])
      {
        nalpha += nr[r];

        for (j = 0; j < nr[r]; j++)
        {
          d = wr[r][nd * j + i];
          if (ar[r].alpha[2] == 0)
          {
            d /= sr[r][j];
          }
          else
          {
            d /= prior_pick_sigma(sqrt((ar[r].alpha[2] * (sr[r][j] * sr[r][j]) + (d * d) / (ad * ad)) / (ar[r].alpha[2] + 1)), ar[r].alpha[2] + 1);
          }
          sum += d * d;
        }
      }
    }
    //  printf("Bom14 Bom14 End %d \n",r);
    nprec = nalpha / sum;

    adj[i] = prior_pick_sigma(1 / sqrt(nprec), nalpha);
  }
}

/* EVALUATE POTENTIAL ENERGY AND ITS GRADIENT. */

void mc_error_calculate(double *error_mean,
                        mc_dynamic_state *ds, /* Current dynamical state */
                        int N_approx,         /* Number of gradient approximations in use */
                        int w_approx,         /* Which approximation to use this time */
                        double *energy,       /* Place to store energy, null if not required */
                        mc_value *gr          /* Place to store gradient, null if not required */
)
{

  double error_compute = 0;
  double exam_error = 0;
  int i, low, high;
  double *res = chk_alloc(1, sizeof(double));
  low = (N_train * (w_approx - 1)) / N_approx;
  high = (N_train * w_approx) / N_approx;
  int eventsNumber = 0;

  for (i = (energy ? 0 : low); i < (energy ? N_train : high); i++)
  {
    // net_sigmas sigmas, *s = &sigmas;
    net_params params, *w = &params;
    // model_specification *m=0;
    model_survival *sv = 0;

    // s->total_sigmas = net_setup_sigma_count(arch,flgs,model);
    w->total_params = net_setup_param_count(arch, flgs);
    // printf("For start 2 \n");

    // net_values *val = ;
    net_model_guess(&train_values[i], res,
                    arch, flgs, model, sv, w, &sigmas, 0);
    // printf("event mean = %f \n", res[0]);
    // error_compute = error_compute+(train_targets[i]- res[0])*(train_targets[i]- res[0]);
    if (train_targets[i] == 0)

      error_compute = error_compute + res[0] * res[0];
    else
      error_compute = error_compute + (1 - res[0]) * (1 - res[0]);

    net_model_guess(&train_values[i], res,
                    arch, flgs, model, sv, w, &sigmas, 0);
    // printf("event mean = %f \n", res[0]);
    // error_compute = error_compute+(train_targets[i]- res[0])*(train_targets[i]- res[0]);
    if (train_targets[i] == 0)

      exam_error = exam_error + res[0] * res[0];
    else
      exam_error = exam_error + (1 - res[0]) * (1 - res[0]);

    eventsNumber = i;
  }
  error_mean[0] = sqrt(error_compute / (eventsNumber * eventsNumber));
  error_mean[0] = sqrt(error_compute / eventsNumber);
  free(res);
  // printf("error = %f \n",error_mean[0]);
}

void mc_app_energy(
    mc_dynamic_state *ds, /* Current dynamical state */
    int N_approx,         /* Number of gradient approximations in use */
    int w_approx,         /* Which approximation to use this time */
    double *energy,       /* Place to store energy, null if not required */
    mc_value *gr          /* Place to store gradient, null if not required */

)
{

  // printf("mc_app_energi started!!!!!!!!!!!!!!!!!\n");
  //  double *res = chk_alloc (1, sizeof (double));
  int binsCount;
  binsCount = 100;
  //  double binsErrorSignal[binsCount];
  //  double binsErrorBackground[binsCount];
  int nbinErrorBackground[binsCount];
  double TP[binsCount];
  double FP[binsCount];
  double SE[binsCount];
  double SP[binsCount];
  double TPExam[binsCount];
  double FPExam[binsCount];
  double SEExam[binsCount];
  double SPExam[binsCount];
  double roc_auc;
  double roc_aucExam;
  int nbinErrorSignal[binsCount];
  int nbinErrorBackgroundExam[binsCount];
  int nbinErrorSignalExam[binsCount];
  double binsError[binsCount];
  double log_prob, inv_temp, log_prob_ex;
  double *energy_ex;
  int i, low, high;
  double error_compute = 0;
  double exam_error = 0;
  int eventsNumber = 0;
  int testNumber = 0;
  double logprobValue = 0;

  inv_temp = !ds->temp_state ? 1 : ds->temp_state->inv_temp;

  int nbin = 0;
  for (nbin = 0; nbin < binsCount; nbin++)
  {
    nbinErrorSignal[nbin] = 0;
    nbinErrorBackground[nbin] = 0;
    nbinErrorSignalExam[nbin] = 0;
    nbinErrorBackgroundExam[nbin] = 0;
    TP[nbin] = 0;
    FP[nbin] = 0;
    SE[nbin] = 0;
    SP[nbin] = 0;
    TPExam[nbin] = 0;
    FPExam[nbin] = 0;
    SEExam[nbin] = 0;
    SPExam[nbin] = 0;
  }
  //	printf("binsCount %d",binsCount);
  if (gr && gr != grad.param_block)
  {
    grad.param_block = gr;
    net_setup_param_pointers(&grad, arch, flgs);
  }

  if (inv_temp >= 0)
  {
    net_prior_prob(&params, &sigmas, &log_prob, gr ? &grad : 0,
                   arch, flgs, priors, 2);
  }
  else
  {
    log_prob = 0;
    if (gr)
    {
      for (i = 0; i < ds->dim; i++)
      {
        gr[i] = 0;
      }
    }
    inv_temp = -inv_temp;
  }

  if (energy)
    *energy = -log_prob;

  if (-log_prob >= 1e30)
  {
    if (energy)
      *energy = 1e30;
    if (gr)
    {
      for (i = 0; i < ds->dim; i++)
        gr[i] = 0;
    }
    return;
  }

  if (inv_temp != 0 && (data_spec != 0 || quadratic_approx))
  {
    if (N_approx > 1 && gr)
    {
      for (i = 0; i < ds->dim; i++)
        gr[i] /= N_approx;
    }

    if (inv_temp != 1 && gr)
    {
      for (i = 0; i < ds->dim; i++)
        gr[i] /= inv_temp;
    }

    if (quadratic_approx)
    {
      double *b, *V;
      int i, j;

      if (energy)
        *energy += *quadratic_approx;

      b = quadratic_approx + 1;
      V = quadratic_approx + 1 + ds->dim;

      for (i = 0; i < ds->dim; i++)
      {
        for (j = 0; j < ds->dim; j++)

        {
          if (energy)
          {
            *energy += (ds->q[i] - b[i]) * (ds->q[j] - b[j]) * *V / 2;
            //*energy_ex += (ds->q[i]-b[i]) * (ds->q[j]-b[j]) * *V / 2;
          }
          if (gr)
          {
            gr[i] += (ds->q[j] - b[j]) * *V / 2;
            gr[j] += (ds->q[i] - b[i]) * *V / 2;
          }
          V += 1;
        }
      }
    }

    else /* Not approximated */
    {
      low = (N_train * (w_approx - 1)) / N_approx;
      high = (N_train * w_approx) / N_approx;

      for (i = (energy ? 0 : low); i < (energy ? N_train : high); i++)
      {
        //  printf("For start  \n");
        if (model->type == 'V'           /* Handle piecewise-constant hazard    */
            && surv->hazard_type == 'P') /*   model specially                   */
        {
          double ot, ft, t0, t1;
          int censored;
          int w;

          if (inv_temp != 1)
          {
            fprintf(stderr,
                    "Can't handle tempering with piecewise-constant hazard models\n");
            exit(1);
          }

          if (train_targets[i] < 0)
          {
            censored = 1;
            ot = -train_targets[i];
          }
          else
          {
            censored = 0;
            ot = train_targets[i];
          }

          t0 = 0;
          t1 = surv->time[0];
          train_values[i].i[0] = surv->log_time ? log(t1) : t1;

          w = 0;

          int iPass = 0;
          for (;;)
          {

            net_func(&train_values[i], 0, arch, flgs, &params);

            ft = ot > t1 ? -(t1 - t0) : censored ? -(ot - t0)
                                                 : (ot - t0);

            net_model_prob(&train_values[i], &ft,
                           &log_prob, gr ? &deriv[i] : 0, arch, model, surv,
                           &sigmas, Cheap_energy);
            // printf("AAAA \n");

            if (energy)
              *energy -= inv_temp * log_prob;

            if (gr && i >= low && i < high)
            {
              net_back(&train_values[i], &deriv[i], arch->has_ti ? -1 : 0,
                       arch, flgs, &params);
              net_grad(&grad, &params, &train_values[i], &deriv[i],
                       arch, flgs);
            }

            // printf("ot %f    t1 %t1 \n",ot,t1);
            if (ot <= t1)
              break;

            t0 = t1;
            w += 1;

            if (surv->time[w] == 0)
            {
              t1 = ot;
              train_values[i].i[0] = surv->log_time ? log(t0) : t0;
            }
            else
            {
              t1 = surv->time[w];
              train_values[i].i[0] = surv->log_time ? (log(t0) + log(t1)) / 2
                                                    : (t0 + t1) / 2;
            }
          }
        }

        else /* Everything except piecewise-constant hazard model */
        {
          net_func(&train_values[i], 0, arch, flgs, &params);
          //   printf("For start  \n");
          // Here the log-probability for the current training case is calculated
          // printf("1\n");
          net_model_prob(&train_values[i], train_targets + data_spec->N_targets * i,
                         &log_prob, gr ? &deriv[i] : 0, arch, model, surv,
                         &sigmas, Cheap_energy);

          // net_model_prob(&test_values[i], test_targets+data_spec->N_targets*i,
          //        &log_prob_ex, gr ? &deriv[i] : 0, arch, model, surv,
          //        &sigmas, Cheap_energy);

          // printf("BBBBB \n");
          // net_sigmas sigmas, *s = &sigmas;
          // net_params params, *w = &params;
          // model_specification *m=0;
          model_survival *sv = 0;

          // s->total_sigmas = net_setup_sigma_count(arch,flgs,model);
          // w->total_params = net_setup_param_count(arch,flgs);
          // printf("For start 2 \n");
          double *res = chk_alloc(1, sizeof(double));
          // net_values *val = ;
          net_model_guess(&train_values[i], res, arch, flgs, model, sv, &params, &sigmas, 0);

          int nbin = 0;
          // double error_eq ;
          // printf("res =%f \n",res[0]);
          ///		binsCount=100;
          for (nbin = 0; nbin < binsCount; nbin++)
          {
            double low = (double)nbin / binsCount;
            double hight = (double)nbin / binsCount + (double)1 / binsCount;
            //			printf("res = %f %f %f  %d \n ",res[0],low,hight, nbin);

            if ((res[0] > low) && (res[0] <= hight))
            {
              if (train_targets[i] == 1)
                nbinErrorSignal[nbin]++;
              else
                nbinErrorBackground[nbin]++;
            }
          }
          //			getchar();

          error_compute = error_compute + (train_targets[i] - res[0]) * (train_targets[i] - res[0]);
          if (i < N_test)

          {
            net_func(&test_values[i], 0, arch, flgs, &params);
            //   printf("For start  \n");
            // Here the log-probability for the current training case is calculated
            // printf("1\n");
            // net_model_prob(&test_values[i], test_targets+data_spec->N_targets*i,
            //  &log_prob, gr ? &deriv[i] : 0, arch, model, surv,
            //  &sigmas, Cheap_energy);
            // free(res);

            //*res = chk_alloc (1, sizeof (double));
            // net_values *val = ;

            net_model_guess(&test_values[i], res,
                            arch, flgs, model, sv, &params, &sigmas, 0);

            testNumber = i;
            for (nbin = 0; nbin < binsCount; nbin++)
            {
              double low = (double)nbin / binsCount;
              double hight = (double)nbin / binsCount + (double)1 / binsCount;
              //		printf("res = %f %f %f  %d \n ",res[0],low,hight, nbin);

              if ((res[0] > low) && (res[0] <= hight))
              {
                if (test_targets[i] == 1)
                  nbinErrorSignalExam[nbin]++;
                else
                  nbinErrorBackgroundExam[nbin]++;
              }

              // printf("res =%f \n",res[0]);

              // d	ouble error_eq ;
            }
            exam_error = exam_error + (test_targets[i] - res[0]) * (test_targets[i] - res[0]);
          }
          free(res);
          // getchar();

          // Correct for the case weight (note that it's been checked to have sence
          // for binary models only!)
          if (data_spec->has_weights)
          {
            // printf(" 21 \n");
            //  printf("For start4  \n");

            if (energy)
            {
              //  printf("For start6  \n");

              *energy -= inv_temp * log_prob * train_weights[i];
              //  getchar();
              // *energy_ex -= inv_temp * log_prob_ex * test_weights[i];

              // printf("energi = %f \n",inv_temp * log_prob * train_weights[i]);
            }
            if (gr && i >= low && i < high)
            {
              // printf("3\n");
              net_values *val = &train_values[i];
              //  printf("For start7  \n");
              // getchar();
              // printf("rez = %f  \n",res[0]);
              net_back(&train_values[i], &deriv[i], arch->has_ti ? -1 : 0, arch, flgs,
                       &params);
              // printf("For start8  \n");
              // printf("train_values= %f \n",train_values[i]);
              net_grad_w(&grad, &params, &train_values[i], &deriv[i], arch, flgs,
                         train_weights[i]);
              // printf("For start9  \n");
            }
          }
          else
          {
            //  printf("For start5  \n");

            if (energy)
            {
              printf("5\n");
              //		 getchar();
              *energy -= inv_temp * log_prob;
            }
            if (gr && i >= low && i < high)
            {

              // printf("6\n");

              net_back(&train_values[i], &deriv[i], arch->has_ti ? -1 : 0, arch, flgs,
                       &params);
              net_grad(&grad, &params, &train_values[i], &deriv[i], arch, flgs);
            }
          }
        }

        eventsNumber = i;

        // if ((i %200)==0)
        // printf("Event Target = %f  event value = %f\n",train_targets[i],exp(log_prob));

        // logprobValue = log_prob;
      }
      FILE *fp;
      int nbin = 0;

      int nSignalExam = 0;
      int nSignal = 0;
      int nBackgroundExam = 0;
      int nBackground = 0;
      double significanceExam = 0;
      double significance = 0;
      for (nbin = 0; nbin < binsCount; nbin++)
      {
        //	if ((res[0]>nbin/binsCount)&&(res[0]<nbin/binsCount+1/binsCount))
        //		if (train_targets[i]=1)
        //		 nbinErrorSignal[nbin]++;
        //	else

        //	   nbinErrorBackground[nbin]++;

        if (nbinErrorSignal[nbin] != 0 || nbinErrorBackground[nbin] != 0)
        {
          double sign = nbinErrorSignal[nbin] / sqrt(nbinErrorSignal[nbin] + nbinErrorBackground[nbin]) * nbinErrorSignal[nbin] / sqrt(nbinErrorSignal[nbin] + nbinErrorBackground[nbin]);
          nSignal = nSignal + nbinErrorSignal[nbin];
          nBackground = nBackground + nbinErrorBackground[nbin];
          significance = significance + sign;
        }
        if (nbinErrorSignalExam[nbin] != 0 || nbinErrorBackgroundExam[nbin] != 0)
        {
          //					double signExam = nbinErrorSignalExam[nbin]/sqrt(nbinErrorSignalExam[nbin]+ nbinErrorBackgroundExam[nbin])* nbinErrorSignalExam[nbin]/sqrt(nbinErrorSignalExam[nbin]+ nbinErrorBackgroundExam[nbin]);
          //				nSignalExam = nSignalExam+ nbinErrorSignalExam[nbin];
          //				significanceExam = significanceExam+signExam;

          double signExam = nbinErrorSignalExam[nbin] / sqrt(nbinErrorSignalExam[nbin] + nbinErrorBackgroundExam[nbin]) * nbinErrorSignalExam[nbin] / sqrt(nbinErrorSignalExam[nbin] + nbinErrorBackgroundExam[nbin]);
          nSignalExam = nSignalExam + nbinErrorSignalExam[nbin];
          nBackgroundExam = nBackgroundExam + nbinErrorBackgroundExam[nbin];
          significanceExam = significanceExam + signExam;
          // printf("aaaaa %f  %f \n",significanceExam, signExam );
        }
      }

      for (nbin = 0; nbin < binsCount; nbin++)
      {

        if (nbinErrorSignal[nbin] != 0 || nbinErrorBackground[nbin] != 0)
        {
          // printf("nbinErrorSignal = %d \n",nbinErrorSignal[nbin]);
          // printf("nbinErrorSignalExam = %d \n",nbinErrorBackground[nbin]);
        }
        {
          if (nbin == 0)
          {
            TP[nbin] = (double)nbinErrorSignal[nbin];
            FP[nbin] = (double)nbinErrorBackground[nbin];
            // printf("TF= %f\n",TP[nbin]);
            // printf("TP=%f \n",FP[nbin]);
          }
          else
          {
            TP[nbin] = TP[nbin - 1] + (double)nbinErrorSignal[nbin];
            FP[nbin] = FP[nbin - 1] + (double)nbinErrorBackground[nbin];
            // printf("TF= %f\n",TP[nbin]);
            // printf("TP=%f\n",FP[nbin]);
          }

          SE[nbin] = TP[nbin] / (double)nSignal;
          SP[nbin] = FP[nbin] / (double)nBackground;

          if (TP[nbin] != 0 || FP[nbin] != 0)
          {
            // printf("SE= %f\n",SE[nbin]);
            // printf("SP=%f\n",SP[nbin]);
          }
          // getchar();
        }

        {
          if (nbin == 0)
          {
            TPExam[nbin] = (double)nbinErrorSignalExam[nbin];
            FPExam[nbin] = (double)nbinErrorBackgroundExam[nbin];
          }
          else
          {
            TPExam[nbin] = TPExam[nbin - 1] + (double)nbinErrorSignalExam[nbin];
            FPExam[nbin] = FPExam[nbin - 1] + (double)nbinErrorBackgroundExam[nbin];
          }
          SEExam[nbin] = TPExam[nbin] / (double)nSignalExam;
          SPExam[nbin] = FPExam[nbin] / (double)nBackgroundExam;
        }
      }

      roc_auc = 0;
      roc_aucExam = 0;
      for (nbin = 0; nbin < binsCount; nbin++)
      {
        if (nbin != binsCount - 1)
        {
          roc_auc = roc_auc + (SP[nbin] + SP[nbin + 1]) * (SE[nbin + 1] - SE[nbin]) / 2;
          roc_aucExam = roc_aucExam + (SPExam[nbin] + SPExam[nbin + 1]) * (SEExam[nbin + 1] - SEExam[nbin]) / 2;
        }
        // printf("roc_auc = %f \n",roc_auc);
        // printf("roc_Exam = %f \n",roc_aucExam);
      }
      printf("roc_auc = %f \n", roc_auc);
      printf("roc_Exam = %f \n", roc_aucExam);

      fp = fopen(ds->netFileName, "a+");
      if (energy)
      {
        fprintf(fp, " %s  %f %s", "Error = ", sqrt(error_compute / eventsNumber), "\n");
        fprintf(fp, " %s  %f %s", "ErrE = ", sqrt(exam_error / testNumber), "\n");
        fprintf(fp, " %s  %f %s", "Sign = ", *energy, "\n");
        fprintf(fp, " %s  %f %s", "Roc_auc = ", roc_auc, "\n");
        fprintf(fp, " %s  %f %s", "Roc_Exam = ", roc_aucExam, "\n");
        // fprintf(fp, " %s  %f %s", "SignE = ",sqrt(significanceExam)/sqrt(nSignalExam),"\n" );
      }
      fclose(fp);
      printf("Nsignal = %d \n", nSignal);
      printf("nBackground = %d \n", nBackground);
      // getchar();
      if (energy)
      {
        //*energy = sqrt(significance)/sqrt(nSignal);
        printf("energy =  %f ", *energy);
        fprintf(stderr, "energy =  %f  %f  %f  %f %f \n",
                *energy,
                sqrt(significance) / sqrt(nSignal),
                sqrt(significanceExam) / sqrt(nSignalExam),
                sqrt(error_compute / eventsNumber),
                sqrt(exam_error / testNumber));
      }
      // getchar();
      printf(" Error In mc_app_energy = %f ", sqrt(significance) / sqrt(nSignal));
      printf(" mc_app_energyExam = %f ", sqrt(significanceExam) / sqrt(nSignalExam));
      printf("  mc_app_energyExam = %f \n", sqrt(exam_error / testNumber));
      // getchar();
      // double err
      // double *res = chk_alloc (1, sizeof (double));
      //      mc_error_calculate(res,ds,N_approx,w_approx,energy,gr);
      //      printf("Event Numbers =%d \n",eventsNumber);
      //      printf("hybrid error  %f \n",sqrt(res[0]/(eventsNumber*eventsNumber)));
      error_compute = 0;
      //     // getchar();
    }

    if (N_approx > 1 && gr)
    {
      printf("6 \n");
      for (i = 0; i < ds->dim; i++)
        gr[i] *= N_approx;
    }

    if (inv_temp != 1 && gr)
    {
      printf("7\n");

      for (i = 0; i < ds->dim; i++)
        gr[i] *= inv_temp;
    }
  }
  // getchar();
}

/* SAMPLE FROM DISTRIBUTION AT INVERSE TEMPERATURE OF ZERO.  Returns zero
   if this is not possible. */

int mc_app_zero_gen(mc_dynamic_state *ds /* Current dynamical state */
)
{
  net_prior_generate(&params, &sigmas, arch, flgs, model, priors, 0, 0, 0);

  return 1;
}

/* SET STEPSIZES FOR EACH COORDINATE. */

void mc_app_stepsizes(mc_dynamic_state *ds /* Current dynamical state */
)
{
  double inv_temp, w;
  int i, j, k, l;

  inv_temp = !ds->temp_state ? 1 : ds->temp_state->inv_temp;

  /* Compute second derivatives of minus log likelihood for unit values. */

  net_model_max_second(seconds.o, arch, model, surv, &sigmas);

  if (inv_temp != 1)
  {
    for (i = 0; i < arch->N_outputs; i++)
    {
      seconds.o[i] *= inv_temp;
    }
  }

  for (l = arch->N_layers - 1; l >= 0; l--)
  {
    for (i = 0; i < arch->N_hidden[l]; i++)
    {
      seconds.h[l][i] = 0;

      if (arch->has_ho[l])
      {
        for (j = 0; j < arch->N_outputs; j++)
        {
          w = sigmas.ho[l][i];
          if (sigmas.ao != 0)
            w *= sigmas.ao[j];
          seconds.h[l][i] += (w * w) * seconds.o[j];
        }
      }

      if (l < arch->N_layers - 1 && arch->has_hh[l])
      {
        for (j = 0; j < arch->N_hidden[l + 1]; j++)
        {
          w = sigmas.hh[l][i];
          if (sigmas.ah[l + 1] != 0)
            w *= sigmas.ah[l + 1][j];
          seconds.h[l][i] += (w * w) * seconds.s[l + 1][j];
        }
      }

      switch (flgs == 0 ? Tanh_type : flgs->layer_type[l])
      {
      case Tanh_type:
      case Identity_type:
      {
        seconds.s[l][i] = seconds.h[l][i];
        break;
      }
      case Sin_type:
      {
        seconds.s[l][i] = 4 * seconds.h[l][i];
        break;
      }
      default:
        abort();
      }
    }
  }

  if (arch->has_ti)
  {
    for (i = 0; i < arch->N_inputs; i++)
    {
      seconds.i[i] = 0;

      if (arch->has_io && (flgs == 0 || (flgs->omit[i] & 1) == 0))
      {
        for (j = 0; j < arch->N_outputs; j++)
        {
          w = sigmas.io[i];
          if (sigmas.ao != 0)
            w *= sigmas.ao[j];
          seconds.i[i] += (w * w) * seconds.o[j];
        }
      }

      for (l = 0; l < arch->N_layers; l++)
      {
        printf("Bom15 Bom15 %d \n", l);
        if (arch->has_ih[l] && (flgs == 0 || (flgs->omit[i] & (1 << (l + 1))) == 0))
        {
          for (j = 0; j < arch->N_hidden[l]; j++)
          {
            w = sigmas.ih[l][i];
            if (sigmas.ah[l] != 0)
              w *= sigmas.ah[l][j];
            seconds.i[i] += (w * w) * seconds.s[l][j];
          }
        }
      }
    }
  }

  /* Initialize stepsize variables to second derivatives of minus log prior. */

  net_prior_max_second(&stepsizes, &sigmas, arch, flgs, priors);

  /* Add second derivatives of minus log likelihood to stepsize variables. */

  if (arch->has_ti)
  {
    for (i = 0; i < arch->N_inputs; i++)
    {
      stepsizes.ti[i] += N_train * seconds.i[i];
    }
  }

  for (l = 0; l < arch->N_layers; l++)
  {
    if (arch->has_th[l])
    {
      for (i = 0; i < arch->N_hidden[l]; i++)
      {
        stepsizes.th[l][i] += N_train * seconds.h[l][i];
      }
    }

    if (arch->has_bh[l])
    {
      for (j = 0; j < arch->N_hidden[l]; j++)
      {
        stepsizes.bh[l][j] += N_train * seconds.s[l][j];
      }
    }

    if (arch->has_ih[l])
    {
      k = 0;
      for (i = 0; i < arch->N_inputs; i++)
      {
        if (flgs == 0 || (flgs->omit[i] & (1 << (l + 1))) == 0)
        {
          for (j = 0; j < arch->N_hidden[l]; j++)
          {
            stepsizes.ih[l][k * arch->N_hidden[l] + j] += train_sumsq[i] * seconds.s[l][j];
          }
          k += 1;
        }
      }
    }

    if (l < arch->N_layers - 1 && arch->has_hh[l])
    {
      for (i = 0; i < arch->N_hidden[l]; i++)
      {
        for (j = 0; j < arch->N_hidden[l + 1]; j++)
        {
          stepsizes.hh[l][i * arch->N_hidden[l + 1] + j] += N_train * seconds.s[l + 1][j];
        }
      }
    }

    if (arch->has_ho[l])
    {
      for (i = 0; i < arch->N_hidden[l]; i++)
      {
        for (j = 0; j < arch->N_outputs; j++)
        {
          stepsizes.ho[l][i * arch->N_outputs + j] += N_train * seconds.o[j];
        }
      }
    }
  }

  if (arch->has_io)
  {
    k = 0;
    for (i = 0; i < arch->N_inputs; i++)
    {
      if (flgs == 0 || (flgs->omit[i] & 1) == 0)
      {
        for (j = 0; j < arch->N_outputs; j++)
        {
          stepsizes.io[k * arch->N_outputs + j] += train_sumsq[i] * seconds.o[j];
        }
        k += 1;
      }
    }
  }

  if (arch->has_bo)
  {
    for (j = 0; j < arch->N_outputs; j++)
    {
      stepsizes.bo[j] += N_train * seconds.o[j];
    }
  }

  /* Convert from second derivatives to appropriate stepsizes. */

  for (k = 0; k < ds->dim; k++)
  {
    ds->stepsize[k] = 1 / sqrt(ds->stepsize[k]);
  }
}

/* COMPUTE ADJUSTED SUM OF SQUARES OF A SET OF PARAMETERS. */

static double sum_squares(net_param *wt,  /* Parameters to compute sum of squares for */
                          net_sigma *adj, /* Adjustments, or zero */
                          int n           /* Number of of parameters */
)
{
  double sum_sq, d;
  int i;

  sum_sq = 0;

  if (adj == 0)
  {
    for (i = 0; i < n; i++)
    {
      d = wt[i];
      sum_sq += d * d;
    }
  }
  else
  {
    for (i = 0; i < n; i++)
    {
      d = wt[i] / adj[i];
      sum_sq += d * d;
    }
  }

  return sum_sq;
}

/* RANDOM-GRID METROPOLIS UPDATE FOR AN UPPER SIGMA VALUE.  The value is
   taken in and returned in standard deviation form, but the Metropolis
   update is done in log precision form. */

static double rgrid_sigma(double stepsize, /* Stepsize for update */
                          mc_iter *it,
                          double current, /* Current value of hyperparameter */
                          double width,   /* Width parameter for top-level prior */
                          double alpha0,  /* Alpha for top-level prior */
                          double alpha1,  /* Alpha for lower-level prior */
                          double sum,     /* Sum of lower-level precisions */
                          int n           /* Number of lower-level precision values */
)
{
  double logcur, lognew, U;
  double Ecur, Enew;
  double w, a;

  w = 1 / (width * width);
  a = alpha0 - n * alpha1;

  logcur = -2.0 * log(current);

  Ecur = -logcur * a / 2 + exp(logcur) * alpha0 / (2 * w) + exp(-logcur) * alpha1 * sum / 2;

  U = rand_uniopen() - 0.5;
  lognew = (2 * stepsize) * (U + floor(0.5 + logcur / (2 * stepsize) - U));

  Enew = -lognew * a / 2 + exp(lognew) * alpha0 / (2 * w) + exp(-lognew) * alpha1 * sum / 2;

  it->proposals += 1;
  it->delta = Enew - Ecur;

  U = rand_uniform(); /* Do every time to keep in sync for coupling purposes */

  if (U < exp(-it->delta))
  {
    it->move_point = 1;
    logcur = lognew;
  }
  else
  {
    it->rejects += 1;
    it->move_point = 0;
  }

  return exp(-0.5 * logcur);
}
