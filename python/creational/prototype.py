"""Prototype Pattern.

> Specify the kinds of objects to create using
> a prototypical instance, and create new
> objects by copying this prototype.

This sample uses the pattern on the following
context:
    - An application share data between
        components through a network;
    - The actual format and configuration (i.e.,
        metadata) for a load
        varies greatly and should be changeable
        during runtime;
    - The payload (i.e., the actual data to be
        shared) is always a mapping from keys
        to values compatible with JSON;
"""

import base64
import json
import random
from abc import ABC, abstractmethod
from datetime import datetime


class LoadPrototype(ABC):
    """Data (payload) with metadata."""

    @abstractmethod
    def initialize(self, payload: dict):
        """Initialize the load with the
        payload.
        """

    @abstractmethod
    def send(self):
        """Send the payload."""

    @abstractmethod
    def clone(self) -> "LoadPrototype":
        """Return clone of self."""


class LoadManager:
    """Manager of available
    prototypes for data transmission.
    """

    def __init__(self):
        self._prototypes: dict[str, LoadPrototype] = dict()
        self._default: LoadPrototype = None

    def register(self, prototype: LoadPrototype, key: str, is_default: bool = False):
        """Register new prototype.

        :param prototype: prototype.
        :param key: unique identifier.
        :param is_default: if this is the default prototype
            when `get` is called without a key.
        """
        assert key not in self._prototypes
        self._prototypes[key] = prototype

        if is_default:
            self._default = prototype

    def get(self, key: str = None) -> LoadPrototype:
        if key is None:
            assert self._default is not None
            return self._default

        return self._prototypes[key]


class PlainLoad(LoadPrototype):
    def __init__(self, should_encode: bool = False, payload: dict = None):
        self._should_encode = should_encode
        self._payload = payload

    def initialize(self, payload: dict):
        self._payload = payload

    def send(self):
        assert self._payload is not None
        data = str(self._payload)
        if self._should_encode:
            data = "ENCODED: " + base64.b64encode(data.encode()).decode()
        print(f"[PlainLoad] {data}")

    def clone(self) -> "PlainLoad":
        return PlainLoad(self._should_encode, self._payload)


class HttpLoad(LoadPrototype):
    def __init__(self, url: str, payload: dict = None):
        self._url = url
        self._payload = payload

    def initialize(self, payload: dict):
        self._payload = payload

    def send(self):
        assert self._payload is not None
        data = dict(
            timestamp=datetime.now().isoformat(), version=1.0, data=self._payload
        )
        print(f"[HttpLoad] POST to {self._url} with body:")
        print(json.dumps(data, indent=2, ensure_ascii=False))

    def clone(self) -> "HttpLoad":
        return HttpLoad(self._url, self._payload)


if __name__ == "__main__":
    # Create manager
    registry = LoadManager()

    # Register available prototypes
    registry.register(PlainLoad(should_encode=False), key="plain", is_default=True)
    registry.register(PlainLoad(should_encode=True), key="base64")
    registry.register(HttpLoad("http://localhost:8484"), key="http")

    # Simulate components
    for _ in range(10):
        # Generate random payload
        payload = dict(value=random.randint(0, 1000))

        # Get prototype
        prototype = registry.get(random.choice([None, "plain", "base64", "http"]))

        # Initialize load
        load = prototype.clone()
        load.initialize(payload)

        # Send data
        load.send()
