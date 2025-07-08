/* Factory Method Pattern.
*
* > Define an interface for creating an object,
* > but let subclasses decide which class to
> instantiate (i.e., defer instantiation to
> subclasses).
>
> This sample uses the pattern on the following
> context:
>     - In an event processing engine, a processor
>         is an object that should be run whenever
>         an event happens;
>     - The processor, then, process the event and
>         produces a new event as a response;
>     - The response event should then be handled
>         accordingly;
*/
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <regex>
#include <type_traits>
using namespace std;

struct Runtime_error {
  string message;
};

/*
 * Abstract response.
 * */
class AbstractResponse {
public:
  string data;
  virtual void handle() = 0;
};

/*
 * Abstract processor.
 * */
class AbstractProcessor {
public:
  virtual unique_ptr<AbstractResponse> create_response(string data) = 0;
  virtual string process(string event) = 0;
  virtual string name() = 0;
  void on_event(string event) {
    printf("[%s] Received event: %s\n", this->name().c_str(), event.c_str());
    string data;
    try {
      data = this->process(event);
    } catch (Runtime_error re) {
      data = re.message;
    }

    this->create_response(data)->handle();
  }
};

class LogResponse : public AbstractResponse {
public:
  LogResponse(string data) { this->data = data; }

  void handle() { printf("[LogResponse] data: %s\n", this->data.c_str()); }
};

class HttpNotifyResponse : public AbstractResponse {
private:
  string url = "http://localhost:324";

public:
  HttpNotifyResponse(string data) { this->data = data; }

  HttpNotifyResponse(string data, string url) : url{url} { this->data = data; }

  void handle() {
    printf("[HttpNotifyResponse] sending notification to %s with data '%s'\n",
           this->url.c_str(), this->data.c_str());
  }
};

template <typename T> class SumProcessor : public AbstractProcessor {
  static_assert(is_base_of<AbstractResponse, T>::value,
                "T must be a subclass of AbstractResponse");

public:
  SumProcessor() {}

  unique_ptr<AbstractResponse> create_response(string data) {
    return make_unique<T>(data);
  }

  string name() { return "SumProcessor"; }

  string process(string event) {
    char msg[512] = "";
    regex re("SUM ([0-9]+) \\+ ([0-9]+)");
    smatch match;
    if (!regex_match(event, match, re)) {
      sprintf(msg, "Failed to process event '%s': Unknown event",
              event.c_str());
      throw Runtime_error{msg};
    }
    sprintf(msg, "RESULT %d", stoi(match[1].str()) + stoi(match[2].str()));
    return msg;
  }
};

void test_processor(AbstractProcessor &processor) {
  for (int i = 0; i < 10; i++) {
    bool use_sum_event = random() % 2 == 0;
    char event[512] = "";

    // Create random event data
    if (use_sum_event) {
      sprintf(event, "SUM %d + %d", (int)(random() % 101),
              (int)(random() % 101));
    } else {
      int n_letters = 5 + random() % 6;
      for (int i = 0; i < n_letters; i++) {
        sprintf(event, "%s%c", event,
                (random() % 2 == 0) ? (char)(65 + random() % 26)
                                    : (char)(97 + random() % 26));
      }
    }

    // Test processor
    processor.on_event(event);
  }
}

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG
  srand(time(nullptr));

  // Create an instance of the processor
  bool use_log_response = random() % 2 == 0;
  if (use_log_response) {
    SumProcessor<LogResponse> processor;
    test_processor(processor);
  } else {
    SumProcessor<HttpNotifyResponse> processor;
    test_processor(processor);
  }

  return 0;
}
