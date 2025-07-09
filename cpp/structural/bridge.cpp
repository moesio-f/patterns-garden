/* Bridge Pattern.
 *
 * > Decouple an abstraction from its
 * > implementation so that the two can vary
 * > independently
 *
 * This sample uses the pattern on the following
 * context:
 *     - A benchmark function for optimization
 *        is a mathematical function to be used
 *        by optimization algorithms to assess
 *        their performance;
 *     - In order to implement such a function
 *         a basic set of mathematical operations
 *         is needed;
 *     - Most of the time, those operators occur
 *         on a vector or matrix of values;
 *     - Some of those operations can be implemented
 *         in different ways (e.g., either using the
 *         functions available by the language or by
 *         using external libraries);
 * */
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>
using namespace std;

// === Utilities ===
struct Runtime_error {
  string message;
};

double randomf(double a, double b) {
  double rand_0_to_1 = (double)rand() / (double)RAND_MAX;
  return a + (b - a) * rand_0_to_1;
}

/*
 * Interface for vector operations.
 * */
template <typename T> class VectorOperations {
  static_assert(is_integral<T>::value || is_floating_point<T>::value,
                "T must be of numeric type.");

public:
  virtual vector<T> add(const vector<T> &a, const vector<T> &b) = 0;
  virtual vector<T> sub(const vector<T> &a, const vector<T> &b) = 0;
  virtual vector<T> mul(const vector<T> &a, const vector<T> &b) = 0;
  virtual vector<T> power(const vector<T> &a, T b) = 0;
  virtual T dot(const vector<T> &a, const vector<T> &b) = 0;
  virtual T reduce_sum(const vector<T> &a) = 0;
};

/*
 * Default implementation.
 * */
template <typename T> class DefaultOperations : public VectorOperations<T> {
private:
  void assert_same_shape(const vector<T> &a, const vector<T> &b) {
    if (a.size() != b.size()) {
      throw Runtime_error{"Vectors should have same size."};
    }
  }

public:
  vector<T> add(const vector<T> &a, const vector<T> &b) {
    assert_same_shape(a, b);
    vector<T> out;
    for (int i = 0; i < a.size(); i++) {
      out.push_back(a[i] + b[i]);
    }

    return out;
  }

  vector<T> sub(const vector<T> &a, const vector<T> &b) {
    assert_same_shape(a, b);
    vector<T> out;
    for (int i = 0; i < a.size(); i++) {
      out.push_back(a[i] + b[i]);
    }
    return out;
  }

  vector<T> mul(const vector<T> &a, const vector<T> &b) {
    assert_same_shape(a, b);
    vector<T> out;
    for (int i = 0; i < a.size(); i++) {
      out.push_back(a[i] * b[i]);
    }
    return out;
  }

  vector<T> power(const vector<T> &a, T b) {
    vector<T> out;
    for (int i = 0; i < a.size(); i++) {
      out.push_back(pow(a[i], b));
    }

    return out;
  }

  virtual T dot(const vector<T> &a, const vector<T> &b) {
    assert_same_shape(a, b);
    T sum = 0;
    for (int i = 0; i < a.size(); i++) {
      sum += a[i] * b[i];
    }
    return sum;
  }

  virtual T reduce_sum(const vector<T> &a) {
    T sum = 0;
    for (int i = 0; i < a.size(); i++) {
      sum += a[i];
    }
    return sum;
  }
};

/*
 * Function interface.
 * */
template <typename T> class Function {
protected:
  unique_ptr<VectorOperations<T>> operations;

  Function(unique_ptr<VectorOperations<T>> operations)
      : operations{operations} {}
  Function() : operations{make_unique<DefaultOperations<T>>()} {}

public:
  virtual T operator()(vector<T> x) = 0;
};

/*
 * Sphere function.
 * */
template <typename T> class Sphere : public Function<T> {
public:
  T operator()(vector<T> x) {
    return this->operations->reduce_sum(this->operations->power(x, 2));
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Instantiate the sphere function
  Sphere<double> fn;

  // Test some random values
  for (int i = 0; i < 10; i++) {
    // Get random array
    int size = 1 + random() % 4;
    vector<double> x;
    for (int j = 0; j < size; j++) {
      x.push_back(randomf(0, 10));
    }

    // Call function
    double out = fn(x);

    // Show results
    printf("x: [");
    for (int j = 0; j < x.size(); j++) {
      printf("%.3f", x[j]);
      if (j + 1 < x.size()) {
        printf(", ");
      }
    }
    printf("], f(x): [%.3f]\n", out);
  }

  return 0;
}
