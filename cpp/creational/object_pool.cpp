/*Object Pool Pattern.
 *
 * > Uses a set of initialized objects
 * > kept ready to use, in a "pool",
 * > rather than allocating and destroying
 * > them on demand.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application has a huge set of AI models
 *         that work together to provide a given
 *         functionality;
 *     - Each model is fairly fast to run an inference;
 *     - Loading the models on the other hand requires
 *         the application to initiate a connection with
 *         a remote server that provides the most up-to-date
 *         version of the model;
 *     - Once loaded, the models are used for small periods
 *         of time on each call;
 */
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
using namespace std;

// === Utilities ===
struct Runtime_error {
  string message;
};

/*
 * Model interface.
 * */
template <typename K, typename T> class AbstractModel {
public:
  virtual vector<K> predict(vector<T> x) = 0;
  virtual void reset() {};
};

/*
 * Model pool.
 * */
template <typename K, typename T> class ModelPool {
private:
  vector<shared_ptr<AbstractModel<K, T>>> free, in_use;

public:
  ModelPool() {}

  void add(shared_ptr<AbstractModel<K, T>> model) {
    this->free.push_back(model);
  }

  shared_ptr<AbstractModel<K, T>> get() {
    if (free.empty()) {
      throw Runtime_error{"All objects are in_use or pool is empty."};
    }

    auto model = this->free[this->free.size() - 1];
    this->free.pop_back();
    this->in_use.push_back(model);
    return model;
  }

  void release(shared_ptr<AbstractModel<K, T>> model) {
    bool released = false;

    for (int i = 0; i < this->in_use.size(); i++) {
      auto &obj = this->in_use[i];
      if (model == obj) {
        this->free.push_back(model);
        this->in_use.erase(this->in_use.begin() + i);
        released = true;
      }
    }

    if (!released) {
      throw Runtime_error{"Object not managed by pool."};
    }
  }

  int free_count() { return this->free.size(); }

  int in_use_count() { return this->in_use.size(); }

  void print() {
    cout << "Pool(free=" << this->free_count()
         << ", in_use=" << this->in_use_count() << ")\n";
  }
};

/*
 * Sample model.
 * */
class SampleModel : public AbstractModel<int, int> {
private:
  vector<int> shift;

public:
  SampleModel() {
    for (int i = 0; i < 10; i++) {
      this->shift.push_back(random() % 21 - 10);
    }

    // Simulate slow initialization
    this_thread::sleep_for(chrono::milliseconds(500));
  }

  vector<int> predict(vector<int> x) {
    vector<int> out;
    int size = shift.size(), i = 0;
    auto fn = [size, *this, &i, &out](int v) {
      out.push_back(v + this->shift[i++ % size]);
    };
    for_each(x.begin(), x.end(), fn);
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
  ModelPool<int, int> pool;
  cout << "Empty pool created: ";
  pool.print();

  // Add some models
  cout << "Creating some models...\n";
  for (int i = 0; i < 3; i++) {
    pool.add(make_shared<SampleModel>());
  }
  pool.print();

  // Get some models
  cout << "Locking models...\n";
  vector<shared_ptr<AbstractModel<int, int>>> models{pool.get(), pool.get(),
                                                     pool.get()};
  pool.print();

  // Utility vector print
  int counter = 0;
  auto print = [&counter](int value) {
    if (counter == 0) {
      cout << "[";
    }

    cout << value;

    if (1 + counter++ < 3) {
      cout << ", ";
    } else {
      counter = 0;
      cout << "]";
    }
  };

  // Run predictions
  cout << "Running predictions:\n";
  vector<int> x = {1, 2, 3};
  for (int i = 0; i < models.size(); i++) {
    vector<int> fx = models[i]->predict(x);
    pool.release(models[i]);

    cout << "Model " << i + 1 << "\n\tx=";
    for_each(x.begin(), x.end(), print);
    cout << "\n\tf(x)=";
    for_each(fx.begin(), fx.end(), print);
    cout << "\n\t";
    pool.print();

    if (i + 1 < models.size()) {
      cout << "\n";
    }
  }

  return 0;
}
