/*Adapter/Wrapper Pattern.
 *
 * > Convert the interface of a
 * > class into another interface
 * > clients expect. Adapter lets
 * > classes work together that
 * > couldn't otherwise because of
 * > incompatible interfaces.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application uses multiple third-party
 *         libraries with slight different interfaces
 *         for model training and prediction;
 *     - The application should have a single
 *         interface for any model it uses;
 */
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
using namespace std;

// === Utilities ===
struct Runtime_error {
  string message;
};

/*
 * Numeric model interface.
 * */
template <typename T> class Model {
  static_assert(is_integral<T>::value || is_floating_point<T>::value,
                "T must be of numeric type.");

public:
  virtual void fit(vector<vector<T>> x, vector<int> y) = 0;
  virtual vector<int> predict(vector<vector<T>> x) = 0;
};

/*
 * Adapter for library A.
 * */
template <typename T> class AdapterLibraryA : Model<T> {
private:
  int _min, _max;

public:
  AdapterLibraryA(void *model) : _min{0}, _max{0} {}

  void fit(vector<vector<T>> x, vector<int> y) {
    auto fn = [this](int value) {
      this->_min = min(value, this->_min);
      this->_max = max(value, this->_max);
    };
    for_each(y.begin(), y.end(), fn);
  }

  vector<int> predict(vector<vector<T>> x) {
    vector<int> out;

    // Predict function
    auto fn = [this, &out](vector<T> &value) {
      // Find max-min values for the vector
      auto vec_max = value[0];
      for_each(value.begin(), value.end(),
               [&vec_max](T &element) { vec_max = max(vec_max, element); });

      // Find value to write to out
      int output = min(this->_max, max(this->_min, static_cast<int>(vec_max)));

      // Add to output vector
      out.push_back(output);
    };

    // Apply function and populate out
    for_each(x.begin(), x.end(), fn);

    // Return out
    return out;
  }
};

/*
 * Adapter for library B.
 * */
template <typename T> class AdapterLibraryB : Model<T> {
private:
  int _min, _max;

public:
  AdapterLibraryB(void *model) : _min{0}, _max{0} {}

  void fit(vector<vector<T>> x, vector<int> y) {
    auto fn = [this](int value) {
      this->_min = min(value, this->_min);
      this->_max = max(value, this->_max);
    };
    for_each(y.begin(), y.end(), fn);
  }

  vector<int> predict(vector<vector<T>> x) {
    vector<int> out;

    // Predict function
    auto fn = [this, &out](vector<T> &value) {
      // Find max-min values for the vector
      auto vec_min = value[0];
      for_each(value.begin(), value.end(),
               [&vec_min](T &element) { vec_min = min(vec_min, element); });

      // Find value to write to out
      int output = min(this->_max, max(this->_min, static_cast<int>(vec_min)));

      // Add to output vector
      out.push_back(output);
    };

    // Apply function and populate out
    for_each(x.begin(), x.end(), fn);

    // Return out
    return out;
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Get adapted versions of objects
  AdapterLibraryA<int> model_from_a{nullptr};
  AdapterLibraryB<int> model_from_b{nullptr};

  // Train model
  model_from_a.fit({}, {-1, 0, 1});
  model_from_b.fit({}, {-1, 0, 1});

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

  int counter_vec = 0;
  auto print_vec = [print, &counter_vec](vector<int> &v) {
    if (counter_vec == 0) {
      cout << "[";
    }

    for_each(v.begin(), v.end(), print);

    if (1 + counter_vec++ < 3) {
      cout << ", ";
    } else {
      counter_vec = 0;
      cout << "]";
    }
  };

  // Run some predictions
  cout << "Running predictions:\n";

  // Model from A
  vector<vector<int>> x{{-1, -2, -3}, {-5, -9, 1}, {0, -1, -2}};
  auto fx = model_from_a.predict(x);
  cout << "Adapted model from A: x=";
  for_each(x.begin(), x.end(), print_vec);
  cout << ", f(x)=";
  for_each(fx.begin(), fx.end(), print);
  cout << "\n";

  // Model from B
  fx = model_from_b.predict(x);
  cout << "Adapted model from B: x=";
  for_each(x.begin(), x.end(), print_vec);
  cout << ", f(x)=";
  for_each(fx.begin(), fx.end(), print);
  cout << "\n";

  return 0;
}
