"""Abstract Factory Pattern.

> Provide an interface for creating families
> of related or dependent objects without
> specifying their concrete classes.

This sample uses the pattern on the following
context:
    - An application is to be run on multiple
        operating systems;
    - Families of operating systems require
        different implementations;
    - The application shouldn't be burdened
        on the specifics of how to create each
        specific concrete class;
"""

import random
import time
from abc import ABC, abstractmethod


class AbstractOutput(ABC):
    """Abstract standard output."""

    @abstractmethod
    def write(self, value: str):
        """Writes to this output.

        :param value: value to write.
        """
        ...


class AbstractProcess(ABC):
    """Abstract process.

    Defines the interface for running
    arbitrary commands on different
    processes.
    """

    def __init__(self, command: str):
        self.command = command

    @abstractmethod
    def is_running(self) -> bool: ...

    @abstractmethod
    def start(self): ...

    @abstractmethod
    def id(self) -> int: ...

    @abstractmethod
    def join(self): ...


class AbstractFactory(ABC):
    """Abstract factory for
    processed and standard output.
    """

    def create_process(self, command: str) -> AbstractProcess: ...

    def stdout(self) -> AbstractOutput: ...


class SampleFactory(AbstractFactory):
    """Sample concrete factory."""

    class Output(AbstractOutput):
        """Sample output."""

        def __init__(self, prefix: str):
            self._p = prefix

        def write(self, value: str):
            print(f"[{self._p}] {value}")

    class Process(AbstractProcess):
        """Sample process."""

        def __init__(self, command: str, id: int, stdout: AbstractOutput):
            self.command = command
            self._id = id
            self._stdout = stdout
            self._start, self._end = None, None
            self._duration = random.randint(0, 3)

        def start(self):
            self._stdout.write(
                f"[PID {self.id()}] Process started with command '{self.command}'. "
                f"ETA of {self._duration} seconds."
            )
            self._start = time.perf_counter()

        def is_running(self) -> bool:
            if self._start is None:
                return False

            return self._runtime() < self._duration

        def id(self) -> int:
            return self._id

        def join(self):
            if self._start is None:
                raise ValueError("Process hasn't started.")

            # Wait until process is finished
            to_finish = self._duration - self._runtime()
            if to_finish >= 0:
                time.sleep(to_finish)

        def _runtime(self):
            return time.perf_counter() - self._start

    def __init__(self, os: str):
        self._stdout = self.Output(os)
        self._pid = 1

    def create_process(self, command: str) -> AbstractProcess:
        proc = self.Process(command, self._pid, self.stdout())
        self._pid += 1
        return proc

    def stdout(self) -> AbstractOutput:
        return self._stdout


# Sample concrete factories
WindowsFactory = SampleFactory("Windows")
PosixFactory = SampleFactory("Posix")

if __name__ == "__main__":
    # Runtime selection of singleton factory
    os = random.randint(0, 1)
    factory: AbstractFactory = WindowsFactory if os == 0 else PosixFactory

    # Create instances
    stdout: AbstractOutput = factory.stdout()
    processes: list[AbstractProcess] = [
        factory.create_process(cmd)
        for cmd in ("git status", "git add .", "git diff HEAD")
    ]

    # Inspecting processes
    for p in processes:
        stdout.write(f"Starting process with PID={p.id()} and command='{p.command}'")
        p.start()
        if random.randint(0, 1) == 1:
            stdout.write("Randomly waiting for process to finish :P")
            p.join()

    # Are all processes finished?
    for p in processes:
        stdout.write(
            f"Process with PID={p.id()} is {'running' if p.is_running() else 'finished'}."
        )

    # Wait for any unfinished process
    stdout.write("Waiting for all processes to finish...")
    for p in processes:
        p.join()
    stdout.write("Done!")
