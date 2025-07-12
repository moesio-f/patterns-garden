/* Thread Pool Pattern.
 *
 * > Maintains multiple threads
 * > waiting for tasks to be allocated
 * > for concurrent execution by the
 * > supervising program.
 *
 * This sample uses the pattern on the following
 * context:
 *     - A sorting algorithm of complex items requires
 *        an estimation of the cost for each item;
 *     - The cost estimation is potentially slow;
 *     - The application must be able to deal with
 *        a potentially long list of items to sort;
 * */
#include <deque>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <tuple>
#include <vector>
using namespace std;

/*
 * General-purpose runtime error.
 * */
struct Runtime_error {
  string message;
};

/*
 * Random double.
 * */
double randomf(double a, double b) {
  double rand_0_to_1 = (double)rand() / (double)RAND_MAX;
  return a + (b - a) * rand_0_to_1;
}

/*
 * Complex item.
 * */
class Item {
private:
  // Unique identifier
  const int _id;

  /* More fields */
  // ...

public:
  Item(int id) : _id{id} {}

  const int id() { return this->_id; }

  /* More methods */
  // ...
};

/*
 * Cost estimator.
 * */
class Estimator {
public:
  Estimator() {}

  const double cost(Item &item) {
    // Simulate complex cost estimation
    int random_sleep = static_cast<int>(1000 * randomf(0.8, 1.0));
    this_thread::sleep_for(chrono::milliseconds(random_sleep));

    // For simplicity, generates a random
    // cost based on id
    return randomf(0.0, 1e-3 + item.id() / 2.0);
  }
};

/*
 * Estimator pool.
 * */
class EstimatorPool {
private:
  /*
   * Pool worker. Wraps
   * an estimator for threaded
   * execution.
   * */
  class Worker {
  private:
    int id, processed;
    unique_ptr<Estimator> estimator;
    bool should_end_loop;
    thread th;
    EstimatorPool &pool;

  protected:
    void event_loop() {
      while (!should_end_loop) {
        unique_lock<mutex> lock{pool.queue_mutex};

        if (pool.tasks.size() <= 0) {
          // Release lock
          lock.unlock();

          // Sleep to give chance for new tasks
          //    to be added (should use a monitor
          //    instead of a lock)
          this_thread::sleep_for(chrono::milliseconds(300));
          continue;
        }

        // Get task from FIFO
        auto task = pool.tasks.back();
        pool.tasks.pop_back();

        // Release queue
        lock.unlock();

        // Run estimator and store result
        *get<1>(task) = estimator->cost(*get<0>(task));
        this->processed++;
      }
    }

  public:
    Worker(EstimatorPool &pool, int worker_id, unique_ptr<Estimator> estimator)
        : estimator{std::move(estimator)}, pool{pool}, id{worker_id},
          processed{0}, should_end_loop{false} {}

    void start() {
      this->should_end_loop = false;
      this->th = thread(&EstimatorPool::Worker::event_loop, this);
      this->th.detach();
    }

    void stop() {
      this->should_end_loop = true;
      if (this->th.joinable()) {
        this->th.join();
      }
    }

    int tasks_processed() { return processed; }
  };

  // Pool configuration
  int n_estimators;
  deque<tuple<Item *, double *>> tasks;
  mutex queue_mutex;

  // Workers owned by the pool
  vector<unique_ptr<Worker>> workers;

public:
  EstimatorPool(int n_estimators) : n_estimators{n_estimators} {
    // Create and start workers
    cout << "[EstimatorPool] Initializing workers...\n";
    for (int i = 0; i < n_estimators; i++) {
      workers.push_back(
          make_unique<Worker>(*this, i, make_unique<Estimator>()));
      workers[i]->start();
    }
    cout << "[EstimatorPool] " << n_estimators
         << " workers ready for requests.\n";
  }

  ~EstimatorPool() {
    // Gracefully stop every worker thread
    cout << "[EstimatorPool] Gracefully stopping workers...\n";
    for (auto &worker : workers) {
      worker->stop();
    }
    cout << "[EstimatorPool] All workers stopped.\n";
  }

  void add_task(Item &item, double &cost) {
    unique_lock<mutex> lock{queue_mutex};
    tasks.push_front(make_tuple(&item, &cost));
  }

  unsigned int tasks_to_run() {
    unique_lock<mutex> lock{queue_mutex};
    return tasks.size();
  }

  vector<int> tasks_processed_by_workers() {
    vector<int> out;
    for (auto &worker : workers) {
      out.push_back(worker->tasks_processed());
    }
    return out;
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG
  srand(time(nullptr));

  // Initialize pool
  EstimatorPool pool{4};

  // Flood pool with requests
  vector<Item *> items;
  vector<double *> costs;
  for (int i = 0; i < 20; i++) {
    items.push_back(new Item(i + 1));
    costs.push_back(new double{-1.0});
    pool.add_task(*items[i], *costs[i]);
  }

  // Wait until all requests are handled.
  // Should use futures instead of relying on initial
  //    value of cost.
  int n_results;
  do {
    n_results = 0;
    cout << "[Main] " << pool.tasks_to_run() << " tasks to run.\n";
    for (auto *cost : costs) {
      n_results += *cost != -1.0;
    }
    this_thread::sleep_for(chrono::seconds(1));
  } while (n_results != items.size());
  cout << "[Main] All tasks finished.\n";

  // Show how much tasks each worker processed
  cout << "[Main] Tasks solved by workers: {";
  vector<int> tasks_by_worker = pool.tasks_processed_by_workers();
  for (int i = 0; i < tasks_by_worker.size(); i++) {
    cout << tasks_by_worker[i];
    if (i + 1 < tasks_by_worker.size()) {
      cout << ", ";
    }
  }
  cout << "}\n";

  // Show costs
  cout << setprecision(5);
  for (int i = 0; i < costs.size(); i++) {
    cout << "[Main] Item(id=" << items[i]->id() << ") had cost of ";
    cout << *costs[i] << ".\n";
    delete costs[i];
    delete items[i];
  }

  return 0;
}
