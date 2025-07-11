/* Fluent API Pattern.
 *
 * > An object-oriented API whose design
 * > relies extensively on method
 * > chaining. Its goal is to increase
 * > code legibility by creating a
 * > domain-specific language (DSL).
 *
 * This sample uses the pattern on the following
 * context:
 *     - A very simple customizable calculator
 *        would like to construct the application
 *        during runtime;
 *     - In order to make it readable and easy
 *        to construct, it decides to build
 *        a fluent API;
 * */
#include <deque>
#include <iostream>
#include <map>
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
 * Calculator builder.
 * */
template <typename T> class Calculator {
  static_assert(is_integral<T>::value || is_floating_point<T>::value,
                "T must be of numeric type.");

private:
  enum class OperationKind {
    NONE,
    GREET,
    INPUT,
    PRINT_MESSAGE,
    PRINT_VALUES,
    SUM
  };
  vector<OperationKind> operations;
  deque<string> messages;
  string greeting;
  int available_to_use;

  OperationKind last_operation() { return operations[operations.size() - 1]; }

public:
  Calculator() : greeting{""}, available_to_use{0} {
    // Initial operation is always NONE
    this->operations.push_back(OperationKind::NONE);
  }

  Calculator &greet(string greeting) {
    if (!this->greeting.empty()) {
      throw Runtime_error{"Greet already defined."};
    }

    this->operations.push_back(OperationKind::GREET);
    this->greeting = greeting;
    return *this;
  }

  Calculator &then_read() {
    if (this->last_operation() == OperationKind::NONE) {
      throw Runtime_error{"Greet the user first."};
    }

    auto kind = OperationKind::INPUT;
    this->operations.push_back(kind);
    this->available_to_use++;
    return *this;
  }

  Calculator &then_sum() {
    if (this->available_to_use != 2) {
      throw Runtime_error{"Either not enough/too much values to sum."};
    }

    // Consume 2, return 1
    this->available_to_use -= 1;
    this->operations.push_back(OperationKind::SUM);
    return *this;
  }

  Calculator &then_write(string message) {
    this->operations.push_back(OperationKind::PRINT_MESSAGE);
    this->messages.push_back(message);
    return *this;
  }

  Calculator &then_print_values_with_message(string message) {
    if (this->available_to_use <= 0) {
      throw Runtime_error{"No values to print."};
    }

    // Doesn't consume values
    this->operations.push_back(OperationKind::PRINT_VALUES);
    this->messages.push_back(message);
    return *this;
  }

  void run() {
    // Program variables
    deque<T> values;

    // Utility variables
    int counter{0};
    T input = 0, a = 0, b = 0;

    // Run the program
    for (OperationKind &operation : this->operations) {
      switch (operation) {
      case OperationKind::GREET:
        cout << this->greeting;
        continue;

      case OperationKind::PRINT_MESSAGE:
        // Pop get first element
        cout << this->messages.front();
        this->messages.pop_front();
        continue;

      case OperationKind::PRINT_VALUES:
        // Pop get first element
        cout << this->messages.front();
        this->messages.pop_front();

        // Print all available values
        for (auto &value : values) {
          cout << value;
          if (++counter < values.size()) {
            cout << ", ";
          }
        }

        // Clear variables
        counter = 0;
        cout << "\n";
        continue;

      case OperationKind::INPUT:
        cin >> input;
        values.push_back(input);
        input = 0;
        continue;

      case OperationKind::SUM:
        a = values.front();
        b = values.back();
        values.clear();
        values.push_back(a + b);
        continue;

      default:
        continue;
      }
    }
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize application
  Calculator<double> app;

  // Build and run application
  app.greet("======================\n"
            "Hi there. Welcome to a simple calculator.\n"
            "======================\n")
      .then_write("Type the first value: ")
      .then_read()
      .then_write("Type the second value: ")
      .then_read()
      .then_print_values_with_message("You typed: ")
      .then_sum()
      .then_print_values_with_message("The sum of those values is: ")
      .run();

  return 0;
}
