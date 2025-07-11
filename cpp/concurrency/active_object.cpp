/* Active Object Pattern.
 *
 * > Decouples method execution from
 * > method invocation to enhance
 * > concurrency and simplify synchronized
 * > access to objects that reside in
 * > their own threads of control.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application requires predictions from
 *        an AI model;
 *     - The prediction process might take some time
 *        to process;
 *     - The application can still do other things while
 *        it waits for the predictions;
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
 * Future template.
 * */
template <typename T> class Future {
private:
  bool available;
  T value;

public:
  Future() : available{false} {}

  bool is_available() { return this->available; }

  T &get() { return this->value; }

  void make_available(T value) {
    if (this->available) {
      throw Runtime_error{"Future is already available."};
    }

    this->value = value;
    this->available = true;
  }
};

/*
 * Model object.
 * */
class Model {
private:
  vector<double> parameters;

public:
  Model(int n_parameters) : parameters(n_parameters, 0.0) {}

  void initialize() {
    for (auto &v : parameters) {
      v = randomf(-10.0, 10.0);
    }
  }

  vector<double> predict(vector<double> &x) {
    vector<double> out;
    int idx{0};
    for (auto &v : x) {
      out.push_back(v + this->parameters[idx++ % this->parameters.size()]);
    }
    return out;
  }
};

/*
 * Thread-safe queue for model requests.
 * */
class ActivationQueue {
public:
  class Item {
  public:
    vector<double> &x;
    Future<vector<double>> &out;
    int id;

    Item(vector<double> &x, Future<vector<double>> &out, int id)
        : x{x}, out{out}, id{id} {}
  };

  ActivationQueue() : id{0} {}

  mutex &rm() { return this->_rm; }

  int count() { return this->queue.size(); }

  void insert(vector<double> &x, Future<vector<double>> &out) {
    queue.push_back(Item{x, out, ++id});
  }

  Item pop() {
    auto front = queue.front();
    queue.pop_front();
    return front;
  }

private:
  int id;
  mutex _rm;
  deque<Item> queue;
};

/*
 * Active model object.
 * */
class ActiveModelObject {
private:
  Model model;
  ActivationQueue &queue;
  thread th;
  bool should_stop;

protected:
  void event_loop() {
    cout << "[ActiveModel] Started event loop.\n";

    while (true) {
      // Maybe gracefully stop?
      if (should_stop) {
        break;
      }

      // Acquire lock to queue
      unique_lock<mutex> lock{queue.rm()};

      // Is there data to process?
      if (queue.count() <= 0) {
        // Implicitly release lock
        continue;
      }

      // Read from queue
      auto data = queue.pop();

      // Explicitly release lock after reading
      lock.unlock();

      // Print
      stringstream ss;
      ss << "[ActiveModel] Received new request (ID=" << data.id
         << ") for prediction.\n";
      cout << ss.str();

      // Run prediction
      data.out.make_available(data.x);

      // Simulate slow prediction
      this_thread::sleep_for(chrono::milliseconds(250));

      // Print
      ss.str("");
      ss << "[ActiveModel] Prediction made available for request with ID="
         << data.id << ".\n";
      cout << ss.str();
    }

    cout << "[ActiveModel] Stopped event loop.\n";
  }

public:
  ActiveModelObject(Model model, ActivationQueue &queue)
      : model{model}, queue{queue}, should_stop{false} {}

  void start() {
    if (this->th.joinable()) {
      throw Runtime_error{"Thread already running."};
    }

    // Guarantee it shouldn't stop
    this->should_stop = false;

    // Start new thread with event loop
    this->th = thread(&ActiveModelObject::event_loop, this);
    this->th.detach();
  }

  void stop() {
    this->should_stop = true;
    if (this->th.joinable()) {
      this->th.join();
    }
  }
};

/*
 * Model proxy. For simplicity,
 * it also works as the scheduler.
 * */
class ModelProxy {
private:
  ActivationQueue &queue;

public:
  ModelProxy(ActivationQueue &queue) : queue{queue} {}

  Future<vector<double>> *predict(vector<double> &x) {
    Future<vector<double>> *out = new Future<vector<double>>;
    unique_lock<mutex> lock{queue.rm()};
    queue.insert(x, *out);
    return out;
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG
  srand(time(nullptr));

  // Initialize queue, model, and active model
  // This code should be run only once during
  //    setup.
  ActivationQueue queue;
  Model model(2);
  model.initialize();
  ActiveModelObject active_object{model, queue};

  // Start model
  active_object.start();

  // Get a proxy
  ModelProxy proxy{queue};

  // Run some predictions
  vector<Future<vector<double>> *> results;
  vector<vector<double>> xs{10};
  cout << "[Main] Calling predictions...\n";
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      xs[i].push_back(randomf(0, 500));
    }
    results.push_back(proxy.predict(xs[i]));
  }

  // Wait for all results to become available
  int n_available = 0;
  cout << "[Main] Waiting for results...\n";
  while (n_available != results.size()) {
    n_available = 0;
    for (auto &future : results) {
      if (future->is_available()) {
        n_available += 1;
      }
    }
    stringstream ss;
    ss << "[Main] Currently " << n_available << " results are available.\n";
    cout << ss.str();
    int rand_wait = 1000 * randomf(0.3, 0.7);
    this_thread::sleep_for(chrono::milliseconds(rand_wait));
  }

  // End object
  cout << "[Main] Finishing shared object.\n";
  active_object.stop();
  this_thread::sleep_for(chrono::milliseconds(500));

  // Show predictions
  cout << "[Main] Those are the predictions returned:\n";
  cout << setprecision(6);
  for (auto &future : results) {
    auto &vec = future->get();
    cout << "  [";
    for (int i = 0; i < vec.size(); i++) {
      cout << vec[i];
      if (i + 1 < vec.size()) {
        cout << ", ";
      }
    }
    cout << "]\n";
    delete future;
  }

  return 0;
}
