"""Builder Pattern.

> Separate the construction of a
> complex object from its
> representation so that the same
> construction process can create
> different representations

This sample uses the pattern on the following
context:
    - A Neural Network is a computational model
        used in Machine Learning for arbitrary
        function approximation;
    - Even a simple Feedforward Network can have
        multiple neurons, layers, and activation
        functions;
    - Irrespective of the configuration, the
        network can be called as a function;
"""

import math
import random
from enum import Enum, auto
from typing import Callable, Self


class Activation(Enum):
    """Activation functions.

    :param Linear: identity map.
    :param ReLU: rectified linear unit.
    :param Tanh: hyperbolic tangent.
    """

    Linear = auto()
    ReLU = auto()
    Tanh = auto()


class Layer:
    """Represents a fully connected layer of
    neurons.

    :param n_input: input size.
    :param n_output: output size.
    :param activation: activation function.

    This class abstracts the following process:
        - For each output neuron, <n_input> weights
            are created (non-initialized);
        - On forward pass, each output neuron computes
            activation(sum(input * weight) + b);
        - The output is a list with size <n_output>;
    """

    class Neuron:
        def __init__(self, n_input: int):
            self.parameters = [None] * n_input
            self.bias = None
            self._initialized = False

        def init(self, parameters: list[float], bias: float):
            assert len(parameters) == len(self.parameters)
            self.parameters = parameters
            self.bias = bias
            self._initialized = True

        def __call__(self, x: list[float]) -> float:
            assert self._initialized
            return sum(w * x_ for x_, w in zip(x, self.parameters)) + self.bias

        def __str__(self) -> str:
            return (
                f"{self.__class__.__name__}"
                f"(parameters={len(self.parameters)}, bias=1)"
            )

        def __repr__(self) -> str:
            return str(self)

    def __init__(self, n_input: int, n_output: int, activation: Activation):
        self._activation = activation
        self._activation_fn = self._get_activation(activation)
        self._input = n_input
        self._output = n_output
        self._neurons = [self.Neuron(n_input) for _ in range(n_output)]

    def activation(self) -> Activation:
        return self._activation

    def input_shape(self) -> int:
        return self._input

    def output_shape(self) -> int:
        return self._output

    def parameters(self) -> list[float]:
        return [
            param
            for neuron in self._neurons
            for param in neuron.parameters + [neuron.bias]
        ]

    def intialize(self):
        for n in self._neurons:
            n.init(
                [random.uniform(-1, 1) for _ in range(self._input)],
                random.random(),
            )

    def __call__(self, x: list[float]) -> list[float]:
        return [self._activation_fn(neuron(x)) for neuron in self._neurons]

    def __str__(self) -> str:
        return (
            f"Linear(n_input={self._input}, "
            f"n_output={self._output}, "
            f"parameters={len(self.parameters())}, "
            f"activation={self._activation.name})"
        )

    @staticmethod
    def _get_activation(activation: Activation) -> Callable[[float], float]:
        match activation:
            case Activation.Linear:
                return lambda x: x
            case Activation.ReLU:
                return lambda x: max(0, x)
            case Activation.Tanh:
                return lambda x: (math.exp(x) - math.exp(-x)) / (
                    math.exp(x) + math.exp(-x)
                )


class NeuralNetwork:
    """A simple feedforward sequential
    neural network.

    :param layers: description of each layer.
        Shapes must be compatible.
    """

    def __init__(self, layers: list[Layer], names: list[str] = None):
        if names is None:
            names = [f"Layer {i}" for i in range(1, len(layers) + 1)]
        assert len(names) == len(layers)
        self.layers = layers
        self.names = names

    def parameters(self) -> list[float]:
        return [param for layer in self.layers for param in layer.parameters()]

    def initialize(self):
        for l in self.layers:
            l.intialize()

    def __call__(self, x: list[float]) -> list[float]:
        for l in self.layers:
            x = l(x)
        return x

    def __str__(self) -> str:
        spacing = "=" * 20
        repr = [spacing, "Network Architecture", spacing, "Layers:"]
        repr.extend([f"\t{name}: {l}" for name, l in zip(self.names, self.layers)])
        repr.append(f"Total parameters: {len(self.parameters())}")
        repr.append(spacing)
        return "\n".join(repr)

    def __repr__(self) -> str:
        return str(self)


class NetworkBuilder:
    """Network builder.

    :param input_size: size of inputs.
    """

    def __init__(self, input_size: int):
        self._input = input_size
        self._layers: list[Layer] = []
        self._names = []
        self._counter = dict()

    def add_linear(self, output_size: int) -> Self:
        return self.add_layer(output_size, Activation.Linear)

    def add_relu(self, output_size: int) -> Self:
        return self.add_layer(output_size, Activation.ReLU)

    def add_tanh(self, output_size: int) -> Self:
        return self.add_layer(output_size, Activation.Linear)

    def add_layer(self, output_size: int, activation: Activation) -> Self:
        input_size = self._input
        if len(self._layers) > 0:
            input_size = self._layers[-1].output_shape()
        self._counter[activation] = self._counter.get(activation, 0) + 1
        self._layers.append(Layer(input_size, output_size, activation))
        self._names.append(f"{activation.name} {self._counter[activation]}")
        return self

    def build(self) -> NeuralNetwork:
        return NeuralNetwork(self._layers, names=self._names)


if __name__ == "__main__":
    # Constructing a network
    input_size = random.randint(1, 4)
    builder = NetworkBuilder(input_size)

    # Defining a random architecture
    for _ in range(random.randint(1, 10)):
        layer_size = random.randint(1, 32)
        match random.randint(1, 3):
            case 1:
                builder.add_linear(layer_size)
            case 2:
                builder.add_relu(layer_size)
            case 3:
                builder.add_tanh(layer_size)

    # Make output single value
    builder.add_linear(1)

    # Creating the network
    network = builder.build()

    # Initialize network
    network.initialize()

    # Show architecture
    print(network)

    # Test some random values
    print("Running tests...")
    for _ in range(10):
        x = [round(random.uniform(0, 500), 3) for _ in range(input_size)]
        print(f"x: {x}, f(x): {[round(v, 3) for v in network(x)]}")
