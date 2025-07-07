/* Builder Pattern.
 *
 * > Separate the construction of a
 * > complex object from its
 * > representation so that the same
 * > construction process can create
 * > different representations
 *
 * This sample uses the pattern on the following
 * context:
 *    - A Neural Network is a computational model
 *        used in Machine Learning for arbitrary
 *        function approximation;
 *    - Even a simple Feedforward Network can have
 *        multiple neurons, layers, and activation
 *        functions;
 *    - Irrespective of the configuration, the
 *        network can be called as a function;
 * */
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
using namespace std;

/*
 * Generate random real values in the given interval.
 * */
double randomf(double a, double b) {
  double rand_0_to_1 = (double)rand() / (double)RAND_MAX;
  return a + (b - a) * rand_0_to_1;
}

/*
 * Activation functions.
 * */
namespace Activation {
class Function {
private:
  double (*fn)(double x);
  string name;

public:
  Function(double (*fn)(double x), string name) : fn{fn}, name{name} {}
  double operator()(double x) { return this->fn(x); }
  string str() { return this->name; }
};

double _linear(double x) { return x; }
double _relu(double x) { return (x < 0) ? 0 : x; }
double _tanh(double x) { return tanh(x); };

Function Linear(_linear, "Linear");
Function ReLU(_relu, "ReLU");
Function Tanh(_tanh, "Tanh");
Function *All[3]{&Linear, &ReLU, &Tanh};

} // namespace Activation

/*
 * Neuron.
 * */
class Neuron {
private:
  vector<double> _parameters;

public:
  int input_size;
  Neuron(int input_size) : input_size{input_size} {
    this->_parameters = vector<double>();
    for (int i = 0; i < this->input_size + 1; i++) {
      this->_parameters.push_back(0.0);
    }
  };

  vector<double> &parameters() { return this->_parameters; }

  void initialize() {
    for (auto &param : this->_parameters) {
      param = randomf(-1.0, 1.0);
    }
  }

  double operator()(const vector<double> &x) {
    double sum = this->_parameters[0];
    for (int i = 0; i < x.size(); i++) {
      sum += this->_parameters[i + 1] * x[i];
    }

    return sum;
  }
};

/*
 * Layer. A collection
 * of neurons.
 * */
class Layer {
private:
  vector<Neuron> neurons;
  vector<double *> _parameters;

public:
  int input_size, output_size;
  Activation::Function &activation;

  Layer(int input_size, int output_size, Activation::Function &activation)
      : input_size{input_size}, output_size{output_size},
        activation{activation} {
    for (int i = 0; i < this->output_size; i++) {
      this->neurons.push_back(Neuron(this->input_size));
      for (auto &param : this->neurons[i].parameters()) {
        this->_parameters.push_back(&param);
      }
    }
  }

  void initialize() {
    for (auto &neuron : this->neurons) {
      neuron.initialize();
    }
  }

  vector<double> operator()(const vector<double> &x) {
    vector<double> out = vector<double>();
    for (auto &neuron : this->neurons) {
      out.push_back(this->activation(neuron(x)));
    }
    return out;
  }

  vector<double *> &parameters() { return this->_parameters; }
};

/*
 * Neural network. A collection
 * of sequential layers.
 * */
class NeuralNetwork {
private:
  vector<Layer> layers;
  vector<string> names;
  vector<double *> _parameters;

public:
  NeuralNetwork(vector<Layer> layers, vector<string> names)
      : layers{layers}, names{names} {
    for (auto &layer : this->layers) {
      for (auto *param : layer.parameters()) {
        this->_parameters.push_back(param);
      }
    }
  }

  void initialize() {
    for (auto &layer : this->layers) {
      layer.initialize();
    }
  }

  vector<double> operator()(const vector<double> &x) {
    vector<double> out = x;
    for (auto &layer : this->layers) {
      out = layer(out);
    }
    return out;
  }

  vector<double *> &parameters() { return this->_parameters; }

  void print_architecture() {
    printf("====================\n");
    printf("Network Architecture\n");
    printf("====================\n");
    printf("Layers:\n");
    for (int i = 0; i < this->layers.size(); i++) {
      Layer &layer = this->layers[i];
      string &name = this->names[i];

      printf("\t%s: Layer(n_input=%d, n_output=%d, activation=%s)\n",
             name.c_str(), layer.input_size, layer.output_size,
             layer.activation.str().c_str());
    }
    printf("Total parameters: %ld\n", this->parameters().size());
    printf("====================\n");
  }
};

/*
 * Neural network builder.
 * */
class NetworkBuilder {
private:
  vector<Layer> layers;
  vector<string> names;
  map<string, int> counter;
  int input_size;

  NetworkBuilder &add_layer(int output_size, Activation::Function &activation) {
    int input_size = this->input_size;
    if (this->layers.size() > 0) {
      input_size = this->layers[this->layers.size() - 1].output_size;
    }

    string activation_name = activation.str();
    this->counter[activation_name] += 1;
    this->layers.push_back(Layer(input_size, output_size, activation));

    char name[512];
    sprintf(name, "%s %d", activation_name.c_str(),
            this->counter[activation_name]);
    this->names.push_back(name);
    return *this;
  }

  void reset_counter() {
    for (auto activation : Activation::All) {
      this->counter[activation->str()] = 0;
    }
  }

public:
  NetworkBuilder(int input_size) : input_size{input_size} {
    this->reset_counter();
  }

  NetworkBuilder &add_linear(int output_size) {
    return this->add_layer(output_size, Activation::Linear);
  }

  NetworkBuilder &add_relu(int output_size) {
    return this->add_layer(output_size, Activation::ReLU);
  }

  NetworkBuilder &add_tanh(int output_size) {
    return this->add_layer(output_size, Activation::Tanh);
  }

  NeuralNetwork build() {
    this->reset_counter();
    return NeuralNetwork(this->layers, this->names);
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG and msg buffer
  srand(time(nullptr));

  // Initialize builder
  int input_size = 1 + random() % 4;
  NetworkBuilder builder(input_size);

  // Create random architecture
  int n_layers = 1 + random() % 11;
  for (int i = 0; i < n_layers; i++) {
    int layer_size = 1 + random() % 33;
    switch (random() % 3) {
    case 0:
      builder.add_linear(layer_size);
      break;
    case 1:
      builder.add_relu(layer_size);
      break;
    case 2:
      builder.add_tanh(layer_size);
      break;
    }
  }

  // Make output single value
  builder.add_linear(1);

  // Create network
  NeuralNetwork network = builder.build();

  // Initialize network
  network.initialize();

  // Print architecture
  network.print_architecture();

  // Test some random values
  for (int i = 0; i < 10; i++) {
    // Get random array
    vector<double> x;
    for (int j = 0; j < input_size; j++) {
      x.push_back(randomf(0, 500));
    }

    // Call network
    vector<double> out = network(x);
    printf("x: [");
    for (int j = 0; j < x.size(); j++) {
      printf("%.3f", x[j]);
      if (j + 1 < x.size()) {
        printf(", ");
      }
    }
    printf("], f(x): [%.3f]\n", out[0]);
  }

  return 0;
}
