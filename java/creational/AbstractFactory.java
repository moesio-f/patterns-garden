
/* Abstract Factory Pattern.
 *
 * > Provide an interface for creating families
 * > of related or dependent objects without
 * > specifying their concrete classes.
 *
 * This sample uses the pattern on the following
 * context:
 *    - An application is to be run on multiple
 *        operating systems;
 *    - Families of operating systems require
 *        different implementations;
 *    - The application shouldn't be burdened
 *        on the specifics of how to create each
 *        specific concrete class;
 * */
import java.time.*;
import java.util.Random;

/**
 * Abstract output.
 */
interface AbstractOutput {
    void write(String msg);
}

/**
 * Abstract process.
 */
interface AbstractProcess {

    String command();

    Boolean isRunning();

    void start();

    int id();

    void join();
}

/**
 * Abstract factory.
 */
interface AbstractFactory {
    AbstractProcess createProcess(String command);

    AbstractOutput stdout();
}

/**
 * Example of a concrete output.
 */
class SampleOutput implements AbstractOutput {
    private String prefix;

    public SampleOutput(String prefix) {
        this.prefix = prefix;
    }

    public void write(String msg) {
        System.out.printf("[%s] %s\n", this.prefix, msg);
    }
}

/*
 * Example of a concrete process.
 */
class SampleProcess implements AbstractProcess {
    private int id, duration;
    private AbstractOutput stdout;
    private Instant start;
    private String command;

    public SampleProcess(String command, int id, AbstractOutput stdout) {
        this.id = id;
        this.command = command;
        this.duration = 1 + new Random().nextInt(4);
        this.start = null;
        this.stdout = stdout;
    }

    public String command() {
        return this.command;
    }

    public int id() {
        return this.id;
    }

    public Boolean isRunning() {
        return this.runtime() < this.duration;
    }

    public void start() {
        this.stdout.write(String.format("[PID %d] Process started with command '%s'. ETA of %d seconds.",
                this.id, this.command, this.duration));
        this.start = Instant.now();
    }

    public void join() {
        int toSleep = this.duration - this.runtime();
        if (toSleep > 0) {
            try {
                Thread.sleep(1000 * toSleep);
            } catch (InterruptedException e) {
                // Silent ignore.
            }
        }
    }

    private int runtime() {
        return (int) Duration.between(this.start, Instant.now()).toSeconds();
    }
}

/*
 * Example of concrete factory.
 */
class SampleFactory implements AbstractFactory {
    private SampleOutput stdout;
    private int pid;

    public SampleFactory(String os) {
        this.stdout = new SampleOutput(os);
        this.pid = 1;
    }

    public AbstractOutput stdout() {
        return this.stdout;
    }

    public AbstractProcess createProcess(String command) {
        return new SampleProcess(command, this.pid++, this.stdout);
    }
}

/*
 * Sample execution.
 */
class Main {
    public static void main(String[] args) {
        // Initialize RNG
        Random rng = new Random();

        // Select a factory in runtime
        AbstractFactory factory = new SampleFactory(rng.nextInt(2) == 0 ? "Windows" : "POSIX");

        // Create instances
        AbstractOutput stdout = factory.stdout();
        AbstractProcess[] processes = { factory.createProcess("git status"), factory.createProcess("git add ."),
                factory.createProcess("git diff HEAD") };

        // Inspecting processes
        for (var proc : processes) {
            stdout.write(String.format("Starting process with PID=%d and command='%s'.",
                    proc.id(), proc.command()));
            proc.start();
            if (rng.nextInt(2) == 1) {
                stdout.write("Randomly waiting for process to finish :P");
                proc.join();
            }
        }

        // Are all processes finished?
        for (var proc : processes) {
            stdout.write(String.format("Process with PID=%d is %s.",
                    proc.id(), proc.isRunning() ? "running" : "finished"));
        }

        // Wait for any unfinished process
        stdout.write("Waiting for all processes to finish...");
        for (var proc : processes) {
            proc.join();
        }
        stdout.write("Done!");
    }
}
