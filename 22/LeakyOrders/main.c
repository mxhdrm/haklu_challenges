#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>

#define DEBUG 1
#undef DEBUG

#define ROUNDS 15
#define TIMEOUT 1000 // ns

int game_token[3] = {0x00};

// https://www.softprayog.in/programming/posix-real-time-signals-in-linux
void handle_0(int signum, siginfo_t *siginfo, void *context);
void handle_1(int signum, siginfo_t *siginfo, void *context);
void handle_2(int signum, siginfo_t *siginfo, void *context);

int *gen_signal_list(unsigned int num, int realtime, int *exclude);
struct sigaction *
register_signal_handles(int *signals, unsigned int num_signals,
                        void (*FP_HANDLES[3])(int, siginfo_t *, void *));

int create_fifo(char *pipe_name);
int win(char *flag_path, int fifo_fd);

int main(int argc, char **argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  // BEGIN SETUP
  unsigned int NUM_SIGNALS = 3;
  pid_t pid = getpid();
  char *fifo = "cashline";
  char *flag = "test.flag";
  int fifo_fd;

  char fifo_buff[0xff] = {0x00};
  unsigned int rounds_won = 0;
#ifdef DEBUG
  printf("PID = %d\n", pid);
#endif

  if (!mkfifo(fifo, 0666)) {
    perror("Error creating fifo device");
    return EXIT_FAILURE;
  }

  void (*FP_HANDLES[3])(int, siginfo_t *, void *) = {handle_0, handle_1,
                                                     handle_2};
  int excludes_for_standard[] = {SIGINT, SIGTERM, SIGSTOP};
  // END SETUP

  // BEGIN MAIN_LOOP
  while (rounds_won < ROUNDS) {
    int *signals = gen_signal_list(NUM_SIGNALS, 1, excludes_for_standard);
    if (signals == NULL) {
      printf("Error generating signals\n");
      return EXIT_FAILURE;
    }

    struct sigaction *sigs =
        register_signal_handles(signals, NUM_SIGNALS, FP_HANDLES);
    if (sigs == NULL) {
      perror("Error generating signal handles");
      if (signals != NULL)
        free(signals);
      return EXIT_FAILURE;
    }

    printf("%d %d %d\n", signals[0], signals[1], signals[2]);

    // After the timeout is over, check if all signals were triggered
    sleep(TIMEOUT);
    if (game_token[0] == signals[0] && game_token[1] == signals[1] &&
        game_token[2] == signals[2]) {
      rounds_won++;
      game_token[0] = 0x00;
      game_token[1] = 0x00;
      game_token[2] = 0x00;
    } else {
      free(signals);
      free(sigs);
      return EXIT_FAILURE;
    }

    free(signals);
    free(sigs);
  }

  return win(flag, fifo_fd);
}

int *gen_signal_list(unsigned int num, int realtime, int *exclude) {
  int *signals = calloc(num, sizeof(int));
  unsigned int rand_signal;
  srand(time(NULL));

  // only use random numbers from SIGRTMIN to SIGRTMAX
  if (realtime) {
    for (int i = 0; i < num; i++) {
      signals[i] = (rand() % (SIGRTMAX - SIGRTMIN + 1)) + SIGRTMIN;
#ifdef DEBUG
      printf("REALTIME Generated random signal %d\n", signals[i]);
#endif
    }
  } else {
    // use standard signals, but ignore a list of signals
    int i = 0;
    while (i < num) {
      rand_signal = (rand() % (SIGRTMAX - 1)) + 1;
#ifdef DEBUG
      printf("Generated random signal %d\n", rand_signal);
#endif
      for (int s = 0; s < sizeof(exclude); i++) {
        if (rand_signal == exclude[s]) {
          break;
        }
      }
      signals[i] = rand_signal;
      i++;
    }
  }

  return signals;
}

struct sigaction *
register_signal_handles(int *signals, unsigned int num_signals,
                        void (*FP_HANDLES[3])(int, siginfo_t *, void *)) {
  struct sigaction act;
  struct sigaction *sighandles = calloc(num_signals, sizeof(act));

  for (int i = 0; i < num_signals; i++) {
    memset(&act, 0x00, sizeof(act));
    act.sa_sigaction = FP_HANDLES[i];
    act.sa_flags = 4;
    // act.sa_flags = SA_SIGINFO;

    if (sigaction(signals[i], &act, NULL) == -1) {
      perror("Error creating signal handler");
      free(sighandles);
      return NULL;
    }

    void *ret = memcpy(&sighandles[i], &act, sizeof(act));
    if (ret != &sighandles[i]) {
      perror("Error copying handles");
      free(sighandles);
      return NULL;
    }
  }

  return sighandles;
}

void handle_0(int signum, siginfo_t *siginfo, void *context) {
  printf("caught signal %d\n", signum);
  game_token[0] = signum;
}

void handle_1(int signum, siginfo_t *siginfo, void *context) {
  printf("caught signal %d\n", signum);
  game_token[1] = signum;
}

void handle_2(int signum, siginfo_t *siginfo, void *context) {
  printf("caught signal %d\n", signum);
  game_token[2] = signum;
}

int win(char *flag_path, int pipe_fd) {
  setvbuf(stdout, NULL, _IONBF, 0);
  char flag_buf[0xff] = {0x00};
  int flag = open(flag_path, O_RDONLY);

  if (!flag) {
    return EXIT_FAILURE;
  } else {
    read(flag, &flag_buf, 0xff);
    // write(pipe_fd, flag_buf, 0xff);
    printf("%s\n", flag_buf);
    return EXIT_SUCCESS;
  }
}
