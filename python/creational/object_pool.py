"""Object Pool Pattern.

> Uses a set of initialized objects
> kept ready to use, in a "pool",
> rather than allocating and destroying
> them on demand.

This sample uses the pattern on the following
context:
    - An application has a huge set of AI models
        that work together to provide a given
        functionality;
    - Each model is fairly fast to run an inference;
    - Loading the models on the other hand requires
        the application to initiate a connection with
        a remote server that provides the most up-to-date
        version of the model;
    - Once loaded, the models are used for small periods
        of time on each call;
"""

import time
from typing import Generic, TypeVar
import random
from abc import ABC, abstractmethod

T = TypeVar("T")
K = TypeVar("K")


class AbstractModel(ABC, Generic[K, T]):
    """Abstract model interface.

    A model is something that allows
    running predictions.
    """

    @abstractmethod
    def predict(self, x: list[K]) -> list[T]:
        """Run an inference.

        :param x: list of inputs.
        :return: list of outputs.
        """

    @abstractmethod
    def reset(self):
        """Reset state of the model."""


class ModelPool(Generic[K, T]):
    """Model pool."""

    def __init__(self):
        self._objects: dict[str, dict] = dict(free=dict(), in_use=dict())

    def add(self, model: AbstractModel[K, T]):
        key = id(model)
        assert all(key not in self._objects[k] for k in ["free", "in_use"])
        self._objects["free"][key] = model

    def get(self) -> AbstractModel[K, T]:
        key, model = self._objects["free"].popitem()
        self._objects["in_use"][key] = model
        return model

    def release(self, model: AbstractModel[K, T]):
        key = id(model)
        assert key in self._objects["in_use"]
        del self._objects["in_use"][key]
        model.reset()
        self._objects["free"][key] = model

    def free_count(self) -> int:
        return len(self._objects["free"])

    def in_use_count(self) -> int:
        return len(self._objects["in_use"])

    def __str__(self) -> str:
        return f"Pool(free={self.free_count()}, in_use={self.in_use_count()})"


class SampleModel(AbstractModel[int, int]):
    def __init__(self):
        self._shift = [random.randint(-10, 10) for _ in range(10)]

        # Simulate slow initialization
        time.sleep(0.5)

    def predict(self, x: list[int]) -> list[int]:
        return [x_ + self._shift[i % len(self._shift)] for i, x_ in enumerate(x)]

    def reset(self):
        # No-op, stateless model
        pass


if __name__ == "__main__":
    # Initialize pool
    pool = ModelPool[int, int]()
    print(f"Empty pool created: {pool}")

    # Add some models to the pool
    print("Creating some models...")
    for _ in range(3):
        pool.add(SampleModel())

    # Show pool state
    print(pool)

    # Get some models
    print("Locking models...")
    model_1 = pool.get()
    model_2 = pool.get()
    model_3 = pool.get()
    print(pool)

    # Run predictions
    print("Running predictions:")
    for i, m in enumerate([model_1, model_2, model_3]):
        x = [1, 2, 3]
        print(f"Model {i+1}\n\tx={x}, f(x)={m.predict(x)}\n\tReleasing model.")
        pool.release(m)
        print(f"\t{pool}")
        if i + 1 < 3:
            print()
