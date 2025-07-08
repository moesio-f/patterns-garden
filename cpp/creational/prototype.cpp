/* Prototype Pattern.
 *
 * > Specify the kinds of objects to create using
 * > a prototypical instance, and create new
 * > objects by copying this prototype.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application share data between
 *         components through a network;
 *     - The actual format and configuration (i.e.,
 *         metadata) for a load
 *         varies greatly and should be changeable
 *         during runtime;
 *     - The payload (i.e., the actual data to be
 *         shared) is always a mapping from keys
 *         to values compatible with JSON;
 */
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <regex>
#include <type_traits>
using namespace std;

// === Utilities ===
struct Runtime_error {
  string message;
};

template <typename T>
typename std::enable_if<std::is_integral<T>::value, string>::type
map_to_str(map<string, T> value) {
  char msg[1024] = "";
  sprintf(msg, "{\n");
  int i = 0, size = value.size();
  for (const auto &tuple : value) {
    sprintf(msg, "%s  \"%s\": %s", msg, tuple.first.c_str(),
            to_string(tuple.second).c_str());
    if (i++ + 1 < size) {
      sprintf(msg, "%s,", msg);
    }
    sprintf(msg, "%s\n", msg);
  }
  sprintf(msg, "%s}\n", msg);
  return msg;
}

string remove_whitespaces(string value) {
  const auto to_remove1 = remove(value.begin(), value.end(), '\n');
  value.erase(to_remove1, value.end());
  const auto to_remove2 = remove(value.begin(), value.end(), ' ');
  value.erase(to_remove2, value.end());
  return value;
}

string caesar_encode(string value, int shift) {
  transform(value.cbegin(), value.cend(), value.begin(),
            [shift](auto ch) { return ch + shift; });
  return value;
}

/*
 * Data (payload) with metadata.
 * */
template <typename T> class LoadPrototype {
  static_assert(is_integral<T>::value, "T must be of integral type.");

public:
  virtual void initialize(map<string, T> payload) = 0;
  virtual void send() = 0;
  virtual unique_ptr<LoadPrototype<T>> clone() = 0;
};

/*
 * Manager of available
 * prototypes for data transmission.
 * */
template <typename T> class LoadManager {
private:
  map<string, shared_ptr<LoadPrototype<T>>> prototypes;
  shared_ptr<LoadPrototype<T>> _default;

public:
  LoadManager() : _default{nullptr} {}

  void add(shared_ptr<LoadPrototype<T>> prototype, string key) {
    this->add(prototype, key, false);
  }

  void add(shared_ptr<LoadPrototype<T>> prototype, string key,
           bool is_default) {
    if (this->prototypes.count(key) > 0) {
      throw Runtime_error{"Key already exists."};
    }
    this->prototypes[key] = prototype;
    if (is_default) {
      this->_default = prototype;
    }
  }

  shared_ptr<LoadPrototype<T>> get(string key) { return this->prototypes[key]; }
};

template <typename T> class PlainLoad : public LoadPrototype<T> {
private:
  bool should_encode = false;
  map<string, T> payload;

public:
  PlainLoad() {}

  PlainLoad(bool should_encode) : should_encode{should_encode} {}

  void initialize(map<string, T> payload) { this->payload = payload; }

  void send() {
    string base = remove_whitespaces(map_to_str(this->payload));
    if (this->should_encode) {
      base = "ENCODED: " + caesar_encode(base, 10);
    }
    printf("[PlainLoad] %s\n", base.c_str());
  }

  unique_ptr<LoadPrototype<T>> clone() {
    auto self = make_unique<PlainLoad<T>>(this->should_encode);
    self->initialize(this->payload);
    return self;
  }
};

template <typename T> class HttpLoad : public LoadPrototype<T> {
private:
  string url;
  map<string, T> payload;

public:
  HttpLoad(string url) : url{url} {}

  void initialize(map<string, T> payload) { this->payload = payload; }

  void send() {
    string base = map_to_str(this->payload);
    printf("[HttpLoad] POST to %s with body:\n%s\n", this->url.c_str(),
           base.c_str());
  }

  unique_ptr<LoadPrototype<T>> clone() {
    auto self = make_unique<HttpLoad<T>>(this->url);
    self->initialize(this->payload);
    return self;
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG
  srand(time(nullptr));

  // Create manager
  LoadManager<int> registry;

  // Register available prototypes
  registry.add(make_shared<PlainLoad<int>>(), "plain");
  registry.add(make_shared<PlainLoad<int>>(true), "encoded");
  registry.add(make_shared<HttpLoad<int>>("http://localhost:8484"), "http");
  string available[3]{"plain", "encoded", "http"};

  // Simulate components
  for (int i = 0; i < 10; i++) {
    // Get prototype
    auto prototype = registry.get(available[random() % 3]);

    // Initialize load
    auto load = prototype->clone();
    load->initialize({{"value", random() % 1001}});

    // Send data
    load->send();
  }

  return 0;
}
