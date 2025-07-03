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
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
using namespace std;

/*
 * Abstract output.
 * */
class AbstractOutput {
public:
  virtual void write(string msg) = 0;
};

/*
 * Abstract process.
 * */
class AbstractProcess {
public:
  string command;
  virtual bool is_running() = 0;
  virtual void start() = 0;
  virtual int id() = 0;
  virtual void join() = 0;
};

/*
 * Abstract factory.
 * */
class AbstractFactory {
public:
  virtual unique_ptr<AbstractProcess> create_process(string command) = 0;
  virtual AbstractOutput &stdout() = 0;
};

/*
 * Concrete example of Output.
 * */
class SampleOutput : public AbstractOutput {
private:
  string prefix;

public:
  SampleOutput(string prefix) : prefix{prefix} {}
  void write(string msg) override {
    printf("[%s] %s\n", this->prefix.c_str(), msg.c_str());
  }
};

/*
 * Concrete example of Process.
 * */
class SampleProcess : public AbstractProcess {
private:
  int id_, duration;
  AbstractOutput &stdout;
  time_t start_t;

  int runtime() { return floor(difftime(time(NULL), this->start_t)); }

public:
  SampleProcess(string command, int id, AbstractOutput &stdout)
      : id_{id}, stdout{stdout} {
    this->command = command;
    this->duration = 1 + rand() % 3;
    this->start_t = 0;
  }

  bool is_running() override { return this->runtime() < this->duration; }

  void start() override {
    char msg[512];
    sprintf(msg,
            "[PID %d] Process started with command '%s'. ETA of %d seconds.",
            this->id_, this->command.c_str(), this->duration);
    this->stdout.write(msg);
    this->start_t = time(NULL);
  }

  int id() override { return this->id_; }

  void join() override {
    int to_sleep = this->duration - this->runtime();
    if (to_sleep > 0) {
      this_thread::sleep_for(chrono::seconds(to_sleep));
    }
  }
};

/*
 * Concrete example of Factory.
 * */
class SampleFactory : public AbstractFactory {
private:
  int pid;
  SampleOutput stdout_;

public:
  SampleFactory(string os) : pid{1}, stdout_{SampleOutput(os)} {}

  AbstractOutput &stdout() override { return this->stdout_; }

  unique_ptr<AbstractProcess> create_process(string command) override {
    return make_unique<SampleProcess>(command, this->pid++, this->stdout_);
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG and msg buffer
  char msg[512];
  srand(time(NULL));

  // Select runtime factory
  unique_ptr<AbstractFactory> factory =
      make_unique<SampleFactory>(rand() % 2 == 0 ? "Windows" : "POSIX");

  // Create instances
  AbstractOutput &stdout = factory->stdout();
  unique_ptr<AbstractProcess> processes[3] = {
      factory->create_process("git status"),
      factory->create_process("git add ."),
      factory->create_process("git diff HEAD")};

  // Inspecting processes
  for (int i = 0; i < 3; i++) {
    unique_ptr<AbstractProcess> &proc = processes[i];
    sprintf(msg, "Starting process with PID=%d and command='%s'.", proc->id(),
            proc->command.c_str());
    stdout.write(msg);
    proc->start();
    if (rand() % 2 == 1) {
      stdout.write("Randomly waiting for process to finish :P");
      proc->join();
    }
  }

  // Are all processes finished?
  for (int i = 0; i < 3; i++) {
    unique_ptr<AbstractProcess> &proc = processes[i];
    sprintf(msg, "Process with PID=%d is %s.", proc->id(),
            proc->is_running() ? "running" : "finished");
    stdout.write(msg);
  }

  // Wait for any unfinished process
  stdout.write("Waiting for all processes to finish...");
  for (int i = 0; i < 3; i++) {
    unique_ptr<AbstractProcess> &proc = processes[i];
    proc->join();
  }
  stdout.write("Done!");

  return 0;
}
