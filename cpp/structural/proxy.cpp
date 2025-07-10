/* Proxy Pattern.
 *
 * > Provide a surrogate or placeholder
 * > for another object to control
 * > access to it.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application requires access to a protected resource;
 *     - This protected resource could be on the same host as the
 *         application or somewhere over a network;
 *     - The application shouldn't be tied to those specifics details;
 * */
#include <iostream>
#include <memory>
using namespace std;

/*
 * General-purpose runtime error.
 * */
struct Runtime_error {
  string message;
};

/*
 * Interface of protected resource.
 * */
class IResource {
public:
  virtual void run() = 0;
};

/*
 * Sample resource.
 * */
class SampleResource : public IResource {
public:
  void run() { cout << "[SampleResource] Run called!\n"; }
};

/*
 * Proxy.
 * */
class ResourceProxy : public IResource {
private:
  unique_ptr<IResource> resource;

public:
  ResourceProxy() : resource{make_unique<SampleResource>()} {}

  void run() {
    bool is_blocked{static_cast<bool>(random() % 2)};
    if (is_blocked) {
      cout << "[ResourceProxy] Resource is busy. Try again later.\n";
      return;
    }

    this->resource->run();
  }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG
  srand(time(nullptr));

  // Get proxy
  ResourceProxy proxy;

  // Simulate runs
  for (int i = 0; i < 10; i++) {
    proxy.run();
  }

  return 0;
}
