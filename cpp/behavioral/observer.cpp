/* Observer Pattern.
 *
 * > Define a one-to-many
 * > dependency between objects
 * > so that when one object changes
 * > state, all its dependents are
 * > notified and updated automatically.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application has an object whose state
 *        is a vector of numbers;
 *     - From this initial vector, multiple views
 *        and transformations are stored;
 *     - To make the system more decoupled, the main
 *        object doesn't directly know which views
 *        are available;
 *     - The view should automatically updated whenever
 *        the vector is updated;
 * */
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <type_traits>
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
 * Subscriber interface.
 * */
template <typename T> class ISubscriber {
public:
  virtual void update(vector<T> &context) = 0;
};

/*
 * Publisher base class.
 * */
template <typename T> class Publisher {
private:
  list<ISubscriber<T> *> subscribers;

protected:
  virtual vector<T> &get_state() = 0;

public:
  Publisher() {}

  void subscribe(ISubscriber<T> *subscriber) {
    this->subscribers.push_back(subscriber);
    this->subscribers.unique();

    // Notify subscriber of current state
    subscriber->update(this->get_state());
  }

  void unsubscribe(ISubscriber<T> *subscriber) {
    this->subscribers.remove(subscriber);
  }

  void notify() {
    for (auto &sub : this->subscribers) {
      sub->update(this->get_state());
    }
  }
};

/*
 * Vector.
 * */
template <typename T> class Vector : public Publisher<T> {
  static_assert(is_integral<T>::value || is_floating_point<T>::value,
                "T must be of numeric type.");

private:
  vector<T> state;
  vector<T> &get_state() { return this->state; }

public:
  Vector(vector<T> base) : state{base} {}

  void append(T value) {
    this->state.push_back(value);
    this->notify();
  }

  void sum(T value) {
    for (auto &s : this->state) {
      s += value;
    }
    this->notify();
  }
};

/*
 * Generic view of vector.
 * */
template <typename T> class GenericView : public ISubscriber<T> {
private:
  string name;
  vector<T> state;
  T (*fn)(T value);

public:
  GenericView(string name, T (*fn)(T value)) : name{name}, fn{fn} {}

  void update(vector<T> &context) {
    cout << "[" << name << "] Received new context.\n";
    cout << "[" << name << "] New view: {";
    this->state.clear();
    int counter = 0;
    for (auto &v : context) {
      auto new_v = this->fn(v);
      this->state.push_back(new_v);
      cout << new_v;
      if (++counter < context.size()) {
        cout << ", ";
      }
    }
    cout << "}\n";
  }
};

/*
 * Functions to use with
 * GenericView.
 * */
namespace ViewFunctions {
template <typename T>
typename std::enable_if<is_integral<T>::value || is_floating_point<T>::value,
                        T>::type
add_1(T value) {
  return value + 1;
}

template <typename T>
typename std::enable_if<is_integral<T>::value || is_floating_point<T>::value,
                        T>::type
sub_1(T value) {
  return value - 1;
}
} // namespace ViewFunctions

/*
 * Sample execution.
 * */
int main() {
  // Initialize state
  Vector<double> object{{1.5, 2.32, 3.4}};

  // Create views
  GenericView<double> view_a{"ADD_1", ViewFunctions::add_1},
      view_b{"SUB_1", ViewFunctions::sub_1};

  // Subscribe view to object
  object.subscribe(&view_a);
  object.subscribe(&view_b);

  // Manipulate original object
  cout << "======================\n";
  object.sum(5.25);
  cout << "======================\n";
  object.append(-1.25);

  return 0;
}
