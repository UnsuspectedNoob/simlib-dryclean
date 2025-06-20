#include "simlib.h"

/* EVENTS */
#define EVENT_ARRIVAL 1
#define EVENT_END_SIMULATION 2
#define EVENT_DEPART1 3
#define EVENT_DEPART2 4
#define EVENT_DEPART3 5
#define EVENT_DEPART4 6
#define EVENT_DEPART5 7

/* LISTS */

// Queues
#define LIST_S1_Q 1
#define LIST_S2_Q 2
#define LIST_S3_Q 3
#define LIST_S4_JQ 4
#define LIST_S4_PQ 5
#define LIST_S5_Q 6

// / Servers
#define LIST_S1 7
#define LIST_S2 8
#define LIST_S3 9
#define LIST_S4 10
#define LIST_S5 11

/* LIST ATTRIBUTES */
#define ATTR_TIME 1
#define ATTR_SUIT_ID 2
#define ATTR_DAMAGE_FLAG 3

/* MEAN SERVICE TIMES */
#define SERVICE_S1 6
#define SERVICE_S2 4
#define SERVICE_S3 5
#define SERVICE_UNDAMAGED 5
#define SERVICE_DAMAGED 8
#define SERVICE_S5 12

/* RANDOM NUMBER STREAMS */
#define STREAM_S1 1
#define STREAM_S2 2
#define STREAM_S3 3
#define STREAM_S4_UNDAMAGED 4
#define STREAM_S4_DAMAGED 5
#define STREAM_S5 6
#define STREAM_INTERARRIVAL 7
#define STREAM_DAMAGE_JACKET 8
#define STREAM_DAMAGE_PANTS 9

/* STATISTICAL COUNTER VARIABLES */
#define SAMPST_UNDAMAGED 1
#define SAMPST_DAMAGED 2

/* GLOBAL VARIABLES */
int suit_id = 1;
double mean_interarrival, sim_duration;

FILE *infile, *outfile;

/* FUNCTION PROTOTYPES */
void arrive(void);
void depart1(void);
void depart2(void);
void depart3(void);
void reassembly(void);
void depart4(void);
void depart5(void);
void report(void);
void log_filest(FILE *, const char *, int);

int main(void) {
  /* Open input and output files */
  infile = fopen("input.txt", "r");
  outfile = fopen("output.txt", "w");

  /* Read input parameters */
  fscanf(infile, "%lf %lf", &mean_interarrival, &sim_duration);

  /* Initialise simlib */
  init_simlib();

  /* Set maxatr = max(maximum number of attributes per record, 4) */
  maxatr = 4;

  /* Schedule the first arrival */
  event_schedule(sim_time + expon(mean_interarrival, STREAM_INTERARRIVAL),
                 EVENT_ARRIVAL);

  /* Schedule the end of the simulation */
  event_schedule(sim_duration, EVENT_END_SIMULATION);

  /* Run the simulation until it terminates after an end-simulation event (type
   * EVENT_END_SIMULATION) occurs */

  do {
    /* Determine the next event */
    timing();

    /* Invoke the appropriate event function. */
    switch (next_event_type) {
    case EVENT_ARRIVAL:
      arrive();
      break;
    case EVENT_DEPART1:
      depart1();
      break;
    case EVENT_DEPART2:
      depart2();
      break;
    case EVENT_DEPART3:
      depart3();
      break;
    case EVENT_DEPART4:
      depart4();
      break;
    case EVENT_DEPART5:

      depart5();
      break;
    case EVENT_END_SIMULATION:
      report();
      break;
    }

    /* If the event just executed was not the end-simulation event (type
     * EVENT_END_SIMULATION), continue simulating. Otherwise, end the
     * simulation.  */
  } while (next_event_type != EVENT_END_SIMULATION);

  fclose(infile);
  fclose(outfile);

  printf("Simulation complete. You can find the report in "
         "output.txt in the folder you ran this exectuable.");

  return 0;
}

void arrive(void) {

  /* Ready suit attributes for service or queue */
  transfer[ATTR_TIME] = sim_time;
  transfer[ATTR_SUIT_ID] = suit_id++;

  /* Check to see whether server is idle */
  if (list_size[LIST_S1] < 1) {

    /* Server is idle, so place suit into service. */
    list_file(LAST, LIST_S1);
    event_schedule(sim_time + expon(SERVICE_S1, STREAM_S1), EVENT_DEPART1);
  } else {

    /* Server is not idle, so suit has to wait in server queue */
    list_file(LAST, LIST_S1_Q);
  }

  /* Schedule next arrival event */
  event_schedule(sim_time + expon(mean_interarrival, STREAM_INTERARRIVAL),
                 EVENT_ARRIVAL);
}

void depart1(void) {
  /* Remove suit from server 1 */
  list_remove(FIRST, LIST_S1);
  double time = transfer[ATTR_TIME];
  int id = transfer[ATTR_SUIT_ID];

  /* Arrive jacket part into server 2 */
  // is server 2 idle ?
  if (list_size[LIST_S2] == 0) {
    list_file(FIRST, LIST_S2);

    event_schedule(sim_time + expon(SERVICE_S2, STREAM_S2), EVENT_DEPART2);
    /* Arrive pants part into server 3 */
    transfer[ATTR_TIME] = time;
    transfer[ATTR_SUIT_ID] = id;

  } else {

    list_file(LAST, LIST_S2_Q);
  }

  if (list_size[LIST_S3] == 0) { // is server 3 idle ?
    list_file(FIRST, LIST_S3);

    event_schedule(sim_time + expon(SERVICE_S3, STREAM_S3), EVENT_DEPART3);
  } else {
    list_file(LAST, LIST_S3_Q);
  }

  /* Does the server 1 queue have more suits ? */
  if (list_size[LIST_S1_Q] > 0) {

    /* yes, so remove one from the queue and add it into service, then
     * schedule its departure time. */
    list_remove(FIRST, LIST_S1_Q);
    list_file(FIRST, LIST_S1);
    event_schedule(sim_time + expon(SERVICE_S1, STREAM_S1), EVENT_DEPART1);
  }
}

// Departure event from service of server 2
void depart2(void) {

  /* Take jacket out of service, it is given a damage with a probabiility of 5%,
   * and it is added to the queue for jackets of server 4. */
  list_remove(FIRST, LIST_S2);
  transfer[ATTR_DAMAGE_FLAG] =
      uniform(0, 1, STREAM_DAMAGE_JACKET) < 0.05 ? 1 : 0;
  list_file(LAST, LIST_S4_JQ);

  /* We check if server 4 is idle, and if so trigger the reassembly routine. */
  if (list_size[LIST_S4] == 0) {

    reassembly();
  }

  /* Since the jacket has been taken out of service, a check is done to see if
   * there any more jackets are in queue, if there are, the first one is removed
   * from the queue and put into service. Finally, a departure event is
   * scheduled for this jacket. */
  if (list_size[LIST_S2_Q] > 0) {
    list_remove(FIRST, LIST_S2_Q);
    list_file(FIRST, LIST_S2);
    event_schedule(sim_time + expon(SERVICE_S2, STREAM_S2), EVENT_DEPART2);
  }
}

void depart3(void) {
  /* Similarly to the depart2 function, the pants are taken out of service,
   * given a damage with a probabiility of 10%, and added to the queue for pants
   * of server 4. */
  list_remove(FIRST, LIST_S3);
  transfer[ATTR_DAMAGE_FLAG] =
      uniform(0, 1, STREAM_DAMAGE_PANTS) < 0.10 ? 1 : 0;
  list_file(LAST, LIST_S4_PQ);

  /* We also check to see if server 4 is idle, and trigger a reassembly if so.
   */
  if (list_size[LIST_S4] == 0) {

    reassembly();
  }

  /* And lastly if the departing pair of pants leaves a non-empty queue, remove
   * the first pair of pants and place it into service, and schedule a departure
   * event for it.  */
  if (list_size[LIST_S3_Q] > 0) {
    list_remove(FIRST, LIST_S3_Q);
    list_file(FIRST, LIST_S3);
    event_schedule(sim_time + expon(SERVICE_S3, STREAM_S3), EVENT_DEPART3);
  }
}

void reassembly(void) {

  if (head[LIST_S4_JQ] && head[LIST_S4_PQ] &&
      (int)head[LIST_S4_JQ]->value[ATTR_SUIT_ID] ==
          (int)head[LIST_S4_PQ]->value[ATTR_SUIT_ID]) {
    /* If both queues are non-empty, and their heads are the same, a reassembly
     * is possible. Take note that there would be no time that the suit pieces
     * would be out of order. We remove both from each list, check damage, and
     * proceed as needed. */

    list_remove(FIRST, LIST_S4_JQ);

    int damage = (int)transfer[ATTR_DAMAGE_FLAG];
    list_remove(FIRST, LIST_S4_PQ);
    damage = damage || (int)transfer[ATTR_DAMAGE_FLAG];

    transfer[ATTR_DAMAGE_FLAG] = damage;

    /* A service time is chosen, based on at least one of the suit pieces
     * being damaged. */
    double svc_time = damage ? expon(SERVICE_DAMAGED, STREAM_S4_DAMAGED)
                             : expon(SERVICE_UNDAMAGED, STREAM_S4_UNDAMAGED);

    /* The suit is added into service by server 4 and a departure event for
     * this merged suit pieces is scheduled. */
    list_file(FIRST, LIST_S4);

    event_schedule(sim_time + svc_time, EVENT_DEPART4);
  }
}

void depart4(void) {
  /* As usual, we remove the suit from the server */
  list_remove(FIRST, LIST_S4);

  /* If the suit has a damaged part, then server 5 is checked for
   * idleness. If server 5 is idle, then the suit is placed into service, and a
   * departure event for the suit is scheduled. */
  if (transfer[ATTR_DAMAGE_FLAG] == 1) {
    if (list_size[LIST_S5] == 0) {
      list_file(FIRST, LIST_S5);
      event_schedule(sim_time + expon(SERVICE_DAMAGED, STREAM_S4_DAMAGED),
                     EVENT_DEPART5);
    } else {

      list_file(LAST, LIST_S5_Q);
    }
  }

  /* If the suit isn't damaged, the total time spent by the suit in the system
     is logged. */
  else {
    sampst(sim_time - transfer[ATTR_TIME], SAMPST_UNDAMAGED);
  }

  /* A departure from server 4 must invoke reassembly, in order to keep track of
   * matching suit pieces. */
  reassembly();
}

void depart5(void) {
  /* The default departure event. The suit is taken out of customer relations
   * service, and the total time spent for damaged suits is logged.  */
  list_remove(FIRST, LIST_S5);
  sampst(sim_time - transfer[ATTR_TIME], SAMPST_DAMAGED);

  /* As always, if the queue left behind by the exiting suit, is non-empty, then
   * the first suit is taken out of the queue, placed into service, and a
   * departure event scheduled for this suit. */
  if (list_size[LIST_S5_Q] > 0) {
    list_remove(FIRST, LIST_S5_Q);
    list_file(FIRST, LIST_S5);
    event_schedule(sim_time + expon(SERVICE_S5, STREAM_S5), EVENT_DEPART5);
  }
}

void report(void) {
  fprintf(outfile, "\n\n");
  fprintf(outfile,
          "       --------------------------------------------------\n");
  fprintf(outfile,
          "               Dry-cleaning System Simulation Report     \n");
  fprintf(outfile,
          "       --------------------------------------------------\n\n");

  fprintf(outfile, "Mean interarrival time: %.2f minutes\n", mean_interarrival);
  fprintf(outfile, "Simulation duration: %.2f minutes\n\n", sim_duration);

  fprintf(outfile, "System Time for Suits, Undamaged(1) and Damaged(2):\n");
  fprintf(outfile, "------------------------------------------------\n");
  out_sampst(outfile, SAMPST_UNDAMAGED, SAMPST_DAMAGED);

  fprintf(outfile, "\nQueue Lengths and Server Utilization:\n");
  fprintf(outfile, "--------------------------------------\n\n");

  // Queues
  fprintf(outfile, "Queue Statistics:\n");
  log_filest(outfile, "S1_Q", LIST_S1_Q);
  log_filest(outfile, "S2_Q", LIST_S2_Q);
  log_filest(outfile, "S3_Q", LIST_S3_Q);
  log_filest(outfile, "Jacket_Q (S4_JQ)", LIST_S4_JQ);
  log_filest(outfile, "Pants_Q (S4_PQ)", LIST_S4_PQ);
  log_filest(outfile, "S5_Q", LIST_S5_Q);

  // Utilisations for servers.
  fprintf(outfile, "\nServer Utilization (Avg should be ≤ 1):\n");
  log_filest(outfile, "Server 1", LIST_S1);
  log_filest(outfile, "Server 2", LIST_S2);
  log_filest(outfile, "Server 3", LIST_S3);
  log_filest(outfile, "Server 4", LIST_S4);
  log_filest(outfile, "Server 5", LIST_S5);

  fprintf(outfile, "\nTotal suits accepted into system: %d\n", suit_id - 1);

  sampst(0.0, -SAMPST_UNDAMAGED);
  int num_undamaged = transfer[2];
  sampst(0.0, -SAMPST_DAMAGED);
  int num_damaged = transfer[2];
  fprintf(outfile, "Total suits successfully processed: %d\n",
          num_damaged + num_undamaged);
  fprintf(outfile, "Simulation ended at time: %.2f minutes\n", sim_time);
}

// This helper function calls filest for time-avg statistics
void log_filest(FILE *file, const char *label, int list_id) {
  filest(list_id);
  if ((int)transfer[2] < 0) {
    fprintf(file, "%-18s | Avg:  -     | Max:  -\n", label);
  } else {
    fprintf(file, "%-18s | Avg: %6.3f | Max: %d\n", label, transfer[1],
            (int)transfer[2]);
  }
}
