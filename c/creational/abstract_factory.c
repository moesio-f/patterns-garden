/* Abstract Factory Pattern.
 *
 * > Provide an interface for creating families
 * > of related or dependent objects without
 * > specifying their concrete classes.
 *
 * This sample uses the pattern on the following
 * context:
 *    - An application is to be run on multiple
 *        operating systems;
 *    - Families of operating systems require
 *        different implementations;
 *    - The application shouldn't be burdened
 *        on the specifics of how to create each
 *        specific concrete class;
 * */
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// === Abstract Entities ===
typedef enum { WINDOWS, POSIX } OS;

typedef struct {
  void (*write)(char *msg);
} AbstractOutput;

typedef struct {
  char *command;
  void *self;
  bool (*is_running)(void *self);
  void (*start)(void *self);
  int (*id)(void *self);
  void (*join)(void *self);
} AbstractProcess;

typedef struct {
  AbstractProcess *(*create_process)(char *command);
  AbstractOutput *(*stdout)();
} AbstractFactory;

// === Concrete Outputs ===
void windows_write(char *msg) { printf("[Windows] %s\n", msg); }
void posix_write(char *msg) { printf("[POSIX] %s\n", msg); }

AbstractOutput *create_output(void (*write)(char *msg)) {
  AbstractOutput *out = malloc(sizeof(AbstractOutput));
  out->write = write;
  return out;
}

// === Concrete Process ===
typedef struct {
  char *command;
  int id, duration;
  time_t start;
  AbstractOutput *stdout;
} SampleProcess;

SampleProcess *create(char *command, int id, AbstractOutput *stdout) {
  SampleProcess *proc = malloc(sizeof(SampleProcess));
  proc->command = command;
  proc->id = id;
  proc->duration = 1 + rand() % 3;
  proc->stdout = stdout;
  return proc;
}

void start(void *value) {
  SampleProcess *proc = (SampleProcess *)value;
  char msg[512];
  sprintf(msg, "[PID %d] Process started with command '%s'. ETA of %d seconds.",
          proc->id, proc->command, proc->duration);
  proc->stdout->write(msg);
  proc->start = time(NULL);
}

bool is_running(void *value) {
  SampleProcess *proc = (SampleProcess *)value;
  int runtime = floor(difftime(time(NULL), proc->start));
  return runtime < proc->duration;
}

int id(void *value) { return ((SampleProcess *)value)->id; }

void join(void *value) {
  SampleProcess *proc = (SampleProcess *)value;
  int runtime = floor(difftime(time(NULL), proc->start));
  int to_sleep = proc->duration - runtime;
  if (to_sleep > 0) {
    sleep(to_sleep);
  }
}

// === Concrete Factory Getter ===
int WIN_PID = 1, POSIX_PID = 1;

AbstractOutput *windows_stdout() { return create_output(&windows_write); }
AbstractOutput *posix_stdout() { return create_output(&posix_write); }

AbstractProcess *create_process(char *command, int *pid,
                                AbstractOutput *stdout) {
  SampleProcess *inner = create(command, *pid, stdout);
  AbstractProcess *proc = malloc(sizeof(AbstractProcess));
  proc->self = inner;
  proc->command = command;
  proc->is_running = &is_running;
  proc->start = &start;
  proc->join = &join;
  proc->id = &id;
  *pid += 1;
  return proc;
}

AbstractProcess *create_windows_process(char *command) {
  return create_process(command, &WIN_PID, windows_stdout());
}

AbstractProcess *create_posix_process(char *command) {
  return create_process(command, &POSIX_PID, posix_stdout());
}

AbstractFactory *get_factory(OS os) {
  AbstractFactory *factory = malloc(sizeof(AbstractFactory));
  if (os == WINDOWS) {
    factory->create_process = &create_windows_process;
    factory->stdout = &windows_stdout;
  } else {
    factory->create_process = &create_posix_process;
    factory->stdout = &posix_stdout;
  }
  return factory;
}

// === Main ===
int main() {
  char msg[512];

  // Runtime selection of factory
  srand(time(NULL));
  int rng = rand() % 100;
  AbstractFactory *factory = get_factory(rng < 50 ? WINDOWS : POSIX);

  // Create instances
  AbstractOutput *stdout = factory->stdout();
  AbstractProcess *processes[3] = {NULL, NULL, NULL};
  processes[0] = factory->create_process("git status");
  processes[1] = factory->create_process("git add .");
  processes[2] = factory->create_process("git diff HEAD");

  // Inspecting processes
  for (int i = 0; i < 3; i++) {
    AbstractProcess *proc = processes[i];
    sprintf(msg, "Starting process with PID=%d and command='%s'.",
            proc->id(proc->self), proc->command);
    stdout->write(msg);
    proc->start(proc->self);
    if (rand() % 2 == 1) {
      stdout->write("Randomly waiting for process to finish :P");
      proc->join(proc->self);
    }
  }

  // Are all processes finished?
  for (int i = 0; i < 3; i++) {
    AbstractProcess *proc = processes[i];
    sprintf(msg, "Process with PID=%d is %s.", proc->id(proc->self),
            proc->is_running(proc->self) ? "running" : "finished");
    stdout->write(msg);
  }

  // Wait for any unfinished process
  stdout->write("Waiting for all processes to finish...");
  for (int i = 0; i < 3; i++) {
    AbstractProcess *proc = processes[i];
    proc->join(proc->self);
  }
  stdout->write("Done!");

  return 0;
}
