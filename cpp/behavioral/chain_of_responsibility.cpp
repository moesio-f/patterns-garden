/* Chain of Responsibility Pattern.
 *
 * > Avoid coupling the sender of a
 * > request to its receiver by giving
 * > more than one object a chance to
 * > handle the request. Chain the receiving
 * > objects and pass the request along the
 * > chain until an object handles it.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application employs heuristis to deal with
 *         a complex problem;
 *     - To improve performance, simpler heuristics are employed
 *         whenever the problem instance meets some criteria;
 *     - If an heuristic is unable to solve the problem, it should
 *         pass the problem to the next heuristic in line;
 *     - If there are no more heuristics available, it will try
 *         to solve the problem anyway;
 * */
#include <iostream>
#include <memory>
#include <type_traits>
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
 * Interface for a problem handler.
 * */
template <typename Problem, typename Solution> class IProblemHandler {
public:
  virtual Solution solve(Problem &problem) = 0;
  virtual void set_next(IProblemHandler<Problem, Solution> *next) = 0;
};

/*
 * Base handler.
 * */
template <typename Problem, typename Solution>
class BaseHander : public IProblemHandler<Problem, Solution> {
protected:
  IProblemHandler<Problem, Solution> *next;

public:
  BaseHander() : next{nullptr} {}

  BaseHander(IProblemHandler<Problem, Solution> *next) : next{next} {}

  Solution solve(Problem &problem) {
    if (this->next != nullptr) {
      return this->next->solve(problem);
    }

    throw Runtime_error{"Unable to solve this problem."};
  }

  void set_next(IProblemHandler<Problem, Solution> *next) { this->next = next; }
};

/*
 * Math problem operators.
 * */
enum class Operation { SUM, SUB, MUL, DIV };

string str(Operation op) {
  switch (op) {
  case Operation::SUM:
    return "+";
  case Operation::SUB:
    return "-";
  case Operation::MUL:
    return "*";
  case Operation::DIV:
    return "/";
  }

  return "UNK";
}

/*
 * Math problem.
 * */
template <typename T> class MathProblem {
  static_assert(is_integral<T>::value || is_floating_point<T>::value,
                "T must be of numeric type.");

public:
  const T a, b;
  const Operation op;

  MathProblem(T a, T b, Operation op) : a{a}, b{b}, op{op} {}
};

/*
 * Simple handler.
 * */
template <typename T>
class SimpleHandler : public BaseHander<MathProblem<T>, T> {
public:
  T solve(MathProblem<T> &problem) {
    // Supported problems
    switch (problem.op) {
    case Operation::SUM:
      return problem.a + problem.b;
    case Operation::SUB:
      return problem.a - problem.b;
    default:
      break;
    }

    // Try parent
    try {
      return BaseHander<MathProblem<T>, T>::solve(problem);
    } catch (Runtime_error &e) {
      // "Heuristic"
      return problem.a + problem.a - problem.b;
    }
  }
};

/*
 * Multiplication handler.
 * */
template <typename T>
class MultiplicationHandler : public BaseHander<MathProblem<T>, T> {
public:
  T solve(MathProblem<T> &problem) {
    // Support problem
    if (problem.op == Operation::MUL) {
      return problem.a * problem.b;
    }

    // Try from parent
    try {
      return BaseHander<MathProblem<T>, T>::solve(problem);
    } catch (Runtime_error &e) {
      // "Heuristic"
      return problem.a * 0.5 * problem.b;
    }
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG
  srand(time(nullptr));

  // Initialize concrete handlers
  SimpleHandler<double> simple_handler;
  MultiplicationHandler<double> mult_handler;

  // Configure order
  simple_handler.set_next(&mult_handler);

  // Set a "main" handler
  BaseHander<MathProblem<double>, double> handler{&simple_handler};

  // Actual solver
  auto solver = [](MathProblem<double> &problem) {
    double a = problem.a, b = problem.b;
    Operation op = problem.op;

    switch (op) {
    case Operation::SUM:
      return a + b;
    case Operation::SUB:
      return a - b;
    case Operation::MUL:
      return a * b;
    case Operation::DIV:
      return a / b;
    default:
      return 0.0;
    }
  };

  // Run some examples
  for (int i = 0; i < 10; i++) {
    MathProblem<double> problem{randomf(0, 10), randomf(0, 10),
                                static_cast<Operation>(random() % 4)};
    cout << "\nGenerated random problem: ";
    cout << "Problem(" << problem.a << " " << str(problem.op) << " "
         << problem.b << ")\n";
    cout << "Answer: " << handler.solve(problem) << "\n";
    cout << "True Answer: " << solver(problem) << "\n";
  }

  return 0;
}
