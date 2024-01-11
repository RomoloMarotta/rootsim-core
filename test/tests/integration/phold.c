/**
 * @file test/tests/integration/phold.c
 *
 * @brief A simple and stripped phold implementation
 *
 * SPDX-FileCopyrightText: 2008-2022 HPDCS Group <rootsim@googlegroups.com>
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <test.h>

#include <stdlib.h>
#include <stdio.h>
#include <ROOT-Sim.h>
#include <errno.h>
#include <argp.h>         // Provides GNU argp() argument parser

#include <math.h>

// Event types
#define LOOP			8
#define EXTERNAL_LOOP	7

#define TAU 1
#define EVENTS_PER_LP 5


#ifndef LOOKAHEAD
#define LOOKAHEAD 0.0
#endif

#define CLOCKS_PER_US ((unsigned long long) 2200.0)


// This is the events' payload which is exchanged across LPs
typedef struct _event_content_type {
	int size;
} event_content_type;


// LP simulation state
typedef struct _lp_state_type {
	simtime_t lvt;
	unsigned int events;
	unsigned int event_us;
	unsigned int fan_out;
	unsigned int num_events;
	unsigned long long start_ts;
} lp_state_type __attribute__((aligned(64)));





#define LA (50)


typedef struct model_parameters{
	unsigned int event_us;
	unsigned int fan_out;
	unsigned int num_events;
	long long timeout;
}
model_parameters;


model_parameters args;

struct simulation_configuration conf;

error_t model_parse_opt(int key, char *arg, struct argp_state *state){
	(void)state;
	switch(key){
		case 1000:
			args.event_us = atoi(arg);
			break;
		case 1001:
			args.fan_out = atoi(arg);
			break;
		case 1002:
			args.num_events = atoi(arg);
			break;
	}
	return 0;
}

#define CLOCK_READ() __extension__({ \
			unsigned int lo; \
			unsigned int hi; \
			__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi)); \
			(unsigned long long)(((unsigned long long)hi) << 32 | lo); \
			})


void ProcessEvent(lp_id_t me, simtime_t now, unsigned event_type, _unused const void *event_content, unsigned size,
    void *state)
{

	simtime_t timestamp, delta;
	unsigned int 	i, j = 123;
	unsigned long long loops, tmp;
	lp_state_type *state_ptr = (lp_state_type*)state;
	
	(void) me;
	(void) event_content;
	(void) size;
	

	if(state_ptr != NULL)
		state_ptr->lvt = now;
		


	switch(event_type) {
		case LP_FINI:
			break;

		case LP_INIT:
	      state_ptr = rs_malloc(sizeof(lp_state_type));
				if(state_ptr == NULL) {
					printf("malloc failed\n");
					exit(-1);
				}
				// Explicitly tell ROOT-Sim this is our LP's state
        SetState(state_ptr);
				state_ptr->events = 0;
				state_ptr->event_us = args.event_us;
				state_ptr->fan_out = args.fan_out;
				state_ptr->num_events = args.num_events;
				state_ptr->start_ts = CLOCK_READ();
			if(me == 0) {
				printf("Running a traditional loop-based PHOLD benchmark with counter set to %lld, %d total events per LP, lookahead %f\n", state_ptr->event_us*CLOCKS_PER_US, state_ptr->num_events, LOOKAHEAD);
			}
			
			for(i = 0; i < EVENTS_PER_LP; i++) {
				timestamp = (simtime_t) LOOKAHEAD + (TAU * Random());
				ScheduleNewEvent(me, timestamp, LOOP, NULL, 0);
			}

			break;

		case EXTERNAL_LOOP:
		case LOOP:
			tmp = CLOCK_READ();
                        loops = tmp;
                        while( (tmp-loops) < (state_ptr->event_us*CLOCKS_PER_US) ){
                            tmp = CLOCK_READ();
                        }
			if((tmp - loops) < (state_ptr->event_us*CLOCKS_PER_US)) printf("error looping less than required\n");

			state_ptr->events++;

			delta = LOOKAHEAD + Expent(TAU);
			timestamp = now + delta;
#if DEBUG == 1
			if(timestamp < now){
				printf("ERROR: new ts %f smaller than old ts %f\n", timestamp, now);	
			}
#endif

			if(event_type == LOOP)
				ScheduleNewEvent(me, timestamp, LOOP, NULL, 0);

			if(event_type == LOOP )
			{
				for(j=0;j<state_ptr->fan_out;j++){
						delta = LOOKAHEAD + Expent(TAU);
						timestamp = now + delta;
#if DEBUG == 1
						if(timestamp < now){
							printf("ERROR: new ts %f smaller than old ts %f\n", timestamp, now);	
						}
#endif
						ScheduleNewEvent(Random() * conf.lps, timestamp, EXTERNAL_LOOP, NULL, 0);
				}
			}

			if(event_type == LOOP && Random() < 0.2) {
				ScheduleNewEvent(Random() * conf.lps, timestamp, EXTERNAL_LOOP, NULL, 0);
			}

			break;
		default:
			fprintf(stderr, "Unknown event type\n");
			abort();
	}
}

bool CanEnd(_unused lp_id_t me, const void *snapshot)
{

	lp_state_type *state_ptr = (lp_state_type*)snapshot;
	if(args.timeout)
		return (CLOCK_READ()-state_ptr->start_ts)/(CLOCKS_PER_US*1000000LL)>args.timeout;
	return false;
}

struct simulation_configuration conf = {
    .lps = 1024,
    .n_threads = 0,
    .termination_time = 1000000000,
    .gvt_period = 1000,
    .log_level = LOG_INFO,
    .stats_file = "phold_test",
    .ckpt_interval = 0,
    .prng_seed = 0,
    .core_binding = true,
    .serial = false,
    .dispatcher = ProcessEvent,
    .committed = CanEnd,
};

struct argp_option model_options[] = {
  {"event-us",          1000, "TIME",  0, "Event granularity"               , 0 },
  {"fan-out",           1001, "VALUE", 0, "Event fan out"               , 0 },
  {"num-events",        1002, "VALUE", 0, "Number of event per process to end the simulation"               , 0 },
  { 0, 0, 0, 0, 0, 0} 
};

model_parameters args = {
	.event_us = 5,
	.fan_out  = 1,  
	.num_events = 5000,
	.timeout = 0,
};


#define N_CORES_KEY                 'c'
#define N_PROCESSES_KEY             'p'
#define WALLCLOCK_TIMEOUT_KEY       'w' 

// Order of fields: {NAME, KEY, ARG, FLAGS, DOC, GROUP}.
static struct argp_option options[] = {
  {"ncores",              N_CORES_KEY              , "CORES"   ,  0                  ,  "Number of threads to be used"               , 0 },
  {"nprocesses",          N_PROCESSES_KEY          , "LPS"     ,  0                  ,  "Number of simulation objects"               , 0 },
  {"wall-timeout",        WALLCLOCK_TIMEOUT_KEY    , "SECONDS" ,  0                  ,  "End the simulation after SECONDS elapsed"   , 0 },
  { 0, 0, 0, 0, 0, 0} 
};


const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<romolo.marotta@gmail.com>";
static char doc[] = "ROOT-Sim";
static char args_doc[] = "-c CORES -p LPS";  


static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  // Figure out which option we are parsing, and decide
  // how to store it  
  switch (key)
    {
    case N_CORES_KEY:
      conf.n_threads = atoi(arg);
      conf.serial = conf.n_threads == 1;
      break;
    case N_PROCESSES_KEY:
      conf.lps = atoi(arg);
      break;
    case WALLCLOCK_TIMEOUT_KEY:
      args.timeout = atoi(arg);
      break;      

    case ARGP_KEY_END:
      if(conf.n_threads < 1)
      {
        printf("Please provide a valid number of cores to be used for simulation\n");
        argp_usage (state);
      }
      if(conf.lps < 1)
      {
        printf("Please provide a valid number of logical processes to be used for simulation\n");
        argp_usage (state);
      }
      if(args.timeout < 0)
      {
        printf("Please provide a valid number of seconds which the simulation should last\n");
        argp_usage (state);
      }
      break;
    
    default:
      return 0;
    
    }
  return 0;
}




extern char *model_name;
extern error_t model_parse_opt(int key, char *arg, struct argp_state *state);

static struct argp model_argp = { model_options, model_parse_opt, NULL, NULL, 0, 0, 0 };
static struct argp_child param_child[] = {
{&model_argp, 0, "Model specific", 0},
{NULL, 0, NULL, 0}
};

static struct argp argp       = { options,       parse_opt, args_doc, doc, &param_child[0], 0, 0 };


int main(int argn, char **argv)
{
  argp_parse(&argp, argn, argv, 0, NULL, NULL);
  RootsimInit(&conf);
  return RootsimRun();
}
