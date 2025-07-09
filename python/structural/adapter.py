"""Adapter/Wrapper Pattern.

> Convert the interface of a
> class into another interface
> clients expect. Adapter lets
> classes work together that
> couldn't otherwise because of
> incompatible interfaces.

This sample uses the pattern on the following
context:
    - An application uses multiple third-party
        libraries with slight different interfaces
        for model training and prediction;
    - The application should have a single
        interface for any model it uses;
"""

from abc import ABC, abstractmethod
from typing import Generic, TypeVar, Any

T = TypeVar("T", int, float)


class Model(ABC, Generic[T]):
    """Simple numeric model interface."""

    @abstractmethod
    def fit(self, x: list[list[T]], y: list[int]):
        """Train the model.

        :param x: features for each sample,
            should have homogeneous shape.
        :param y: targets for each sample.
        """

    @abstractmethod
    def predict(self, x: list[list[T]]) -> list[int]:
        """Run inference.

        :param x:
        :return:
        """


class AdapterLibraryA(Model[T]):
    """Adapter for objects of a library A."""

    def __init__(self, obj: Any):
        # Real implementation should
        #   store whatever is needed
        #   to implement fit+predict.
        del obj

    def fit(self, x: list[list[T]], y: list[int]):
        pass

    def predict(self, x: list[list[T]]) -> list[int]:
        return [int(max(x_)) for x_ in x]


class AdapterLibraryB(Model[T]):
    """Adapter for objects of a library B."""

    def __init__(self, obj: Any):
        # Real implementation should
        #   store whatever is needed
        #   to implement fit+predict.
        del obj

    def fit(self, x: list[list[T]], y: list[int]):
        pass

    def predict(self, x: list[list[T]]) -> list[int]:
        return [int(min(x_)) for x_ in x]


if __name__ == "__main__":
    # Get adapted versions of objects
    model_from_a = AdapterLibraryA[int](None)
    model_from_b = AdapterLibraryB[float](None)

    # Run some predictions
    print("Running predictions:")
    int_x = [[1, 2, -3], [10, -9, 4], [22, 5, -2]]
    float_x = [list(map(float, x_)) for x_ in int_x]
    print(f"Adapted model from A: x={int_x}, f(x)={model_from_a.predict(int_x)}")
    print(f"Adapted model from B: x={float_x}, f(x)={model_from_b.predict(float_x)}")
