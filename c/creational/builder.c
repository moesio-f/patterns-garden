/* Abstract Factory Pattern.
 *
 *
 * Builder Pattern.
 *
 * > Separate the construction of a
 * > complex object from its
 * > representation so that the same
 * > construction process can create
 * > different representations
 &
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
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// === Utility functions ===
double randomf(double a, double b) {
  double rand_0_to_1 = (double)rand() / (double)RAND_MAX;
  return a + (b - a) * rand_0_to_1;
}

// === Generic Array container ===

typedef struct {
  void *arr;
  int n;
} Array;

// === Activation Function ===
typedef enum { Linear, ReLU, Tanh } Activation;
static const char *ActivationName[] = {"Linear", "ReLU", "Tanh"};

double activation_linear(double x) { return x; }

double activation_relu(double x) { return (x < 0) ? 0 : x; }

double activation_tanh(double x) { return tanh(x); }

// === Neuron ===
typedef struct Neuron {
  int input_size;
  Array *parameters;
  void (*initialize)(struct Neuron *self);
  double (*call)(struct Neuron *self, Array *x);
} Neuron;

void neuron_initialize(Neuron *neuron) {
  double *parameters = (double *)neuron->parameters->arr;
  for (int i = 0; i < neuron->parameters->n; i++) {
    parameters[i] = randomf(-1.0f, 1.0f);
  }
}

double neuron_call(Neuron *neuron, Array *x) {
  double *parameters = neuron->parameters->arr;
  double *xv = x->arr;
  double sum = parameters[0];
  for (int i = 0; i < x->n; i++) {
    sum += parameters[1 + i] * xv[i];
  }
  return sum;
}

Neuron *create_neuron(Neuron *neuron, int input_size) {
  if (neuron == NULL) {
    neuron = malloc(sizeof(Neuron));
  }
  neuron->input_size = input_size;
  neuron->parameters = malloc(sizeof(Array));
  neuron->parameters->n = 1 + input_size;
  neuron->parameters->arr = malloc(neuron->parameters->n * sizeof(double));
  neuron->initialize = &neuron_initialize;
  neuron->call = &neuron_call;
  return neuron;
}

void neuron_free_parameters(Neuron *neuron) {
  free(neuron->parameters->arr);
  free(neuron->parameters);
}

// === Layer ===
typedef struct Layer {
  int input_size, output_size;
  Array *parameters;
  Neuron *neurons;
  Activation activation;
  void (*initialize)(struct Layer *self);
  Array *(*call)(struct Layer *self, Array *x);
} Layer;

void layer_initialize(Layer *layer) {
  for (int i = 0; i < layer->output_size; i++) {
    layer->neurons[i].initialize(layer->neurons + i);
  }
}

Array *layer_call(Layer *layer, Array *x) {
  int n = layer->output_size;
  Array *output = malloc(sizeof(Array));
  output->n = n;
  output->arr = malloc(n * sizeof(double));
  for (int i = 0; i < n; i++) {
    double value = layer->neurons[i].call(layer->neurons + i, x);
    switch (layer->activation) {
    case Linear:
      value = activation_linear(value);
      break;
    case ReLU:
      value = activation_relu(value);
      break;
    case Tanh:
      value = activation_tanh(value);
      break;
    }
    ((double *)output->arr)[i] = value;
  }
  return output;
}

Layer *create_layer(Layer *layer, int input_size, int output_size,
                    Activation activation) {
  if (layer == NULL) {
    layer = malloc(sizeof(Layer));
  }
  layer->input_size = input_size;
  layer->output_size = output_size;
  layer->activation = activation;
  layer->initialize = &layer_initialize;
  layer->call = &layer_call;

  // Create neurons
  Neuron *neurons = malloc(output_size * sizeof(Neuron));
  for (int i = 0; i < output_size; i++) {
    create_neuron(neurons + i, input_size);
  }

  // Create reference to parameters
  Array *parameters = malloc(sizeof(Array));
  parameters->n = (input_size + 1) * output_size;
  double **arr = malloc(parameters->n * sizeof(double *));
  int idx = 0;
  for (int i = 0; i < output_size; i++) {
    for (int j = 0; j < input_size + 1; j++) {
      arr[idx++] = neurons[i].parameters->arr + j;
    }
  }
  parameters->arr = arr;

  // Store parameters and neurons
  layer->parameters = parameters;
  layer->neurons = neurons;
  return layer;
}

void free_layer_components(Layer *layer) {
  // Free neurons
  for (int i = 0; i < layer->output_size; i++) {
    neuron_free_parameters(layer->neurons + i);
  }
  free(layer->neurons);

  // Free parameters
  free(layer->parameters->arr);
  free(layer->parameters);
}

// === Neural Network ===
typedef struct NeuralNetwork {
  Array *layers;
  Array *names;
  Array *parameters;
  void (*initialize)(struct NeuralNetwork *self);
  Array *(*call)(struct NeuralNetwork *self, Array *x);
} NeuralNetwork;

void network_initialize(NeuralNetwork *network) {
  for (int i = 0; i < network->layers->n; i++) {
    Layer *layer = ((Layer *)network->layers->arr) + i;
    layer->initialize(layer);
  }
}

Array *network_call(NeuralNetwork *network, Array *x) {
  Array *output = x;
  Layer *layers = network->layers->arr;
  for (int i = 0; i < network->layers->n; i++) {
    Array *new = layers[i].call(layers + i, output);
    if (i != 0) {
      free(output);
    }
    output = new;
  }
  return output;
}

// === Network Builder ===
typedef struct NetworkBuilder {
  int _input_size, _counter[3];
  Array *_layers;
  Array *_names;
  struct NetworkBuilder *(*add_linear)(struct NetworkBuilder *self,
                                       int output_size);
  struct NetworkBuilder *(*add_relu)(struct NetworkBuilder *self,
                                     int output_size);
  struct NetworkBuilder *(*add_tanh)(struct NetworkBuilder *self,
                                     int output_size);
  NeuralNetwork *(*build)(struct NetworkBuilder *self);
} NetworkBuilder;

NetworkBuilder *add_layer(NetworkBuilder *builder, int output_size,
                          Activation activation) {
  // Find input size
  int input_size = builder->_input_size;
  Layer *layers = builder->_layers->arr;

  if (builder->_layers->n > 0) {
    input_size = layers[builder->_layers->n - 1].output_size;
  }

  // Increase counter
  builder->_counter[activation] += 1;

  // Create new name
  char *name = malloc(512 * sizeof(char));
  sprintf(name, "%s %d", ActivationName[activation],
          builder->_counter[activation]);
  builder->_names->n += 1;
  builder->_names->arr =
      realloc(builder->_names->arr, builder->_names->n * sizeof(char *));
  ((char **)builder->_names->arr)[builder->_names->n - 1] = name;

  // Create new layer
  builder->_layers->n += 1;
  layers = realloc(layers, builder->_layers->n * sizeof(Layer));
  create_layer(layers + builder->_layers->n - 1, input_size, output_size,
               activation);
  builder->_layers->arr = layers;

  // Return same builder
  return builder;
}

NetworkBuilder *add_linear(NetworkBuilder *builder, int output_size) {
  return add_layer(builder, output_size, Linear);
}

NetworkBuilder *add_relu(NetworkBuilder *builder, int output_size) {
  return add_layer(builder, output_size, ReLU);
}

NetworkBuilder *add_tanh(NetworkBuilder *builder, int output_size) {
  return add_layer(builder, output_size, Tanh);
}

NeuralNetwork *build_network(NetworkBuilder *builder) {
  NeuralNetwork *network = malloc(sizeof(NeuralNetwork));

  // Initialize basic parameters and methods
  network->layers = builder->_layers;
  network->names = builder->_names;
  network->call = &network_call;
  network->initialize = &network_initialize;

  // Create reference to parameters
  Array *parameters = malloc(sizeof(Array));
  parameters->n = 0;
  Layer *layers = network->layers->arr;

  // Find total parameter size
  for (int i = 0; i < network->layers->n; i++) {
    parameters->n += layers[i].parameters->n;
  }

  // Allocate total parameters
  double **arr = malloc(parameters->n * sizeof(double *));
  for (int i = 0; i < parameters->n; i++) {
    for (int j = 0; j < network->layers->n; j++) {
      Array *layer_params = layers[j].parameters;
      for (int k = 0; k < layer_params->n; k++) {
        arr[i] = layer_params->arr + k;
      }
    }
  }
  parameters->arr = arr;
  network->parameters = parameters;

  // Return network
  return network;
}

NetworkBuilder *initialize_builder(int input_size) {
  NetworkBuilder *builder = malloc(sizeof(NetworkBuilder));
  builder->_input_size = input_size;
  builder->_counter[Linear] = 0;
  builder->_counter[ReLU] = 0;
  builder->_counter[Tanh] = 0;

  // Create layers array
  builder->_layers = malloc(sizeof(Array));
  builder->_layers->n = 0;
  builder->_layers->arr = NULL;

  // Create names array
  builder->_names = malloc(sizeof(Array));
  builder->_names->n = 0;
  builder->_names->arr = NULL;

  // Add methods
  builder->add_relu = &add_relu;
  builder->add_tanh = &add_tanh;
  builder->add_linear = &add_linear;
  builder->build = &build_network;

  return builder;
}

void free_builder(NetworkBuilder *builder) { free(builder); }

void free_network(NeuralNetwork *network) {
  // Free names
  char **names = network->names->arr;
  for (int i = 0; i < network->names->n; i++) {
    free(names[i]);
  }
  free(network->names);

  // Free layers
  Layer *layers = network->layers->arr;
  for (int i = 0; i < network->layers->n; i++) {
    free_layer_components(layers + i);
  }
  free(layers);

  // Free network
  free(network);
}

// === Main ===
int main() {
  // Update RNG
  srand(time(NULL));

  // Constructing builder
  int input_size = 1 + random() % 4;
  NetworkBuilder *builder = initialize_builder(input_size);

  // Defining a random architecture
  int n_layers = 1 + random() % 11;
  for (int i = 0; i < n_layers; i++) {
    int layer_size = 1 + random() % 33;
    switch (random() % 3) {
    case 0:
      builder->add_linear(builder, layer_size);
      break;
    case 1:
      builder->add_relu(builder, layer_size);
      break;
    case 2:
      builder->add_tanh(builder, layer_size);
      break;
    }
  }

  // Make output single value
  builder->add_linear(builder, 1);

  // Create network
  NeuralNetwork *network = builder->build(builder);

  // Initialize network
  network->initialize(network);

  // Print network architecture
  printf("====================\n");
  printf("Network Architecture\n");
  printf("====================\n");
  printf("Layers:\n");
  for (int i = 0; i < network->layers->n; i++) {
    Layer layer = ((Layer *)network->layers->arr)[i];
    printf("\t%s: Layer(n_input=%d, n_output=%d, activation=%s)\n",
           ((char **)network->names->arr)[i], layer.input_size,
           layer.output_size, ActivationName[layer.activation]);
  }
  printf("Total parameters: %d\n", network->parameters->n);
  printf("====================\n");

  // Test sine random values
  for (int i = 0; i < 10; i++) {
    // Get random array
    Array *x = malloc(sizeof(Array));
    x->n = input_size;
    double *arr = malloc(x->n * sizeof(double));
    for (int j = 0; j < x->n; j++) {
      arr[j] = randomf(0, 500);
    }
    x->arr = arr;

    // Call network
    Array *out = network->call(network, x);
    printf("x: [");
    for (int j = 0; j < x->n; j++) {
      printf("%.3f", ((double *)x->arr)[j]);
      if (j + 1 < x->n) {
        printf(", ");
      }
    }
    printf("], f(x): [%.3f]\n", ((double *)out->arr)[0]);

    // Free memory
    free(out->arr);
    free(x->arr);
    free(x);
    free(out);
  }

  // Free memory
  free_network(network);
  free_builder(builder);

  return 0;
}
