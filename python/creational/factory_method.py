"""Factory Method Pattern.

> Define an interface for creating an object,
> but let subclasses decide which class to
> instantiate (i.e., defer instantiation to
> subclasses).

This sample uses the pattern on the following
context:
    - In an event processing engine, a processor
        is an object that should be run whenever
        an event happens;
    - The processor, then, process the event and
        produces a new event as a response;
    - The response event should then be handled
        accordingly;
"""

import string
import random
import re
from abc import ABC, abstractmethod


class AbstractResponse(ABC):
    """Abstract response.

    :param data: data of this response.
    """

    def __init__(self, data: str):
        self.data = data

    @abstractmethod
    def handle(self):
        """Event handler."""


class AbstractProcessor(ABC):
    """Abstract processor."""

    @abstractmethod
    def create_response(self, data: str) -> AbstractResponse:
        """Create a responde object with the
        data.

        :param data: data of the response.
        :return: response object.
        """

    @abstractmethod
    def process(self, event: str) -> str:
        """Processing rules for the event.

        :param event: event.
        """

    def on_event(self, event: str):
        print(f"[{self.__class__.__name__}] Received event: {event}")

        # Try to process the data
        try:
            processed_data = self.process(event)
        except Exception as e:
            processed_data = f"Failed to process event '{event}': {e}"

        # Create response from data
        response = self.create_response(processed_data)

        # Handle response
        response.handle()


class LogRespone(AbstractResponse):
    def __init__(self, data: str):
        super().__init__(data)

    def handle(self):
        print(f"[{self.__class__.__name__}] data: {self.data}")


class HttpNotifyResponse(AbstractResponse):
    def __init__(self, data: str, url: str = "http://localhost:324"):
        self.url = url
        super().__init__(data)

    def handle(self):
        print(
            f"[{self.__class__.__name__}] sending "
            f"notification to {self.url} with data '{self.data}'"
        )


class SumProcessor(AbstractProcessor):
    def __init__(self, response_cls: type[AbstractResponse]):
        self._response = response_cls

    def create_response(self, data: str) -> AbstractResponse:
        return self._response(data)

    def process(self, event: str) -> str:
        match = re.fullmatch(r"SUM (?P<a>[0-9]+) \+ (?P<b>[0-9]+)", event)
        assert match is not None, "Unknown event"
        return f"RESULT {int(match.group('a')) + int(match.group('b'))}"


if __name__ == "__main__":
    # Create an instance of the processor
    processor: AbstractProcessor = SumProcessor(
        random.choice([LogRespone, HttpNotifyResponse])
    )

    # Simulate events
    for _ in range(10):
        event_kind = random.choice(["random", "sum"])
        match event_kind:
            case "random":
                event = "".join(
                    random.choice(string.ascii_letters)
                    for _ in range(random.randint(5, 10))
                )
            case "sum":
                event = f"SUM {random.randint(0, 100)} + {random.randint(0, 100)}"

        # Process event
        processor.on_event(event)
