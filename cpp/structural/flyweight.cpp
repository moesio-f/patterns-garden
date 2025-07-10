/* Flyweight Pattern.
 *
 * > Use sharing to support
 * > large numbers of fine-grained
 * > objects efficiently.
 *
 * This sample uses the pattern on the following
 * context:
 *     - An application uses NLP techniques to process
 *         texts;
 *     - The application has multiple modules and
 *         components that require a shared functionality
 *         for POS-tagging;
 *     - The application wishes to share the same underlying
 *         POS-taggin model for performance and
 *         homogeneity reasons;
 * */
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <type_traits>
#include <vector>
using namespace std;

/*
 * General-purpose runtime error.
 * */
struct Runtime_error {
  string message;
};

/*
 * POS tags available.
 * */
enum class POSTag { NOUN, VERB, ADJECTIVE, CONJUNCTION, UNKNOWN };

string str(POSTag tag) {
  switch (tag) {
  case POSTag::NOUN:
    return "NOUN";
  case POSTag::VERB:
    return "VERB";
  case POSTag::ADJECTIVE:
    return "ADJ";
  case POSTag::CONJUNCTION:
    return "CONJ";
  default:
    return "UNK";
  }
}

/*
 * Sample POS tagging model.
 * */
class POSTagger {
public:
  vector<string> tokenize(string text) {
    vector<string> tokens;

    // Create word iterator
    regex words_regex("[^\\s]+");
    auto words_begin = sregex_iterator(text.begin(), text.end(), words_regex);
    auto words_end = sregex_iterator();

    // Iterate over matches
    for (auto i = words_begin; i != words_end; ++i) {
      tokens.push_back(i->str());
    }

    return tokens;
  };

  vector<POSTag> get_tags(vector<string> tokens) {
    vector<POSTag> tags;
    for (auto word : tokens) {
      tags.push_back(static_cast<POSTag>(random() % 5));
    }
    return tags;
  };
};

/*
 * Factory for POS-tagging models.
 * */
class TaggerFactory {
private:
  map<string, shared_ptr<POSTagger>> instances;

public:
  TaggerFactory(shared_ptr<POSTagger> model) {
    this->instances["default"] = model;
  }

  void add(shared_ptr<POSTagger> model, string identifier) {
    if (identifier == "default") {
      throw Runtime_error{"Default indentifier is reserved."};
    }

    this->instances[identifier] = model;
  }

  shared_ptr<POSTagger> get(string identifier) {
    if (this->instances.count(identifier) <= 0) {
      throw Runtime_error{"Identifier doesn't exist."};
    }

    return this->instances[identifier];
  }

  shared_ptr<POSTagger> get() { return this->get("default"); }
};

/*
 * A dodcument associates a
 * text (extrinsic state) with
 * a POS model (intrinsic state).
 * */
class Document {
private:
  string _text;
  vector<string> _tokens;
  vector<POSTag> _tags;
  shared_ptr<POSTagger> tagger;

public:
  Document(string text, shared_ptr<POSTagger> tagger)
      : _text{text}, tagger{tagger} {}

  void analyze() {
    _tokens = tagger->tokenize(_text);
    _tags = tagger->get_tags(_tokens);
  }

  const string &text() { return _text; }
  const vector<POSTag> &tags() { return _tags; }
  const vector<string> &tokens() { return _tokens; }
};

/*
 * Sample execution.
 * */
int main() {
  // Initialize RNG
  srand(time(nullptr));

  // Get a factory with a default model
  TaggerFactory factory{make_shared<POSTagger>()};

  // Create documents
  Document doc1{
      "Hi! This is an example.\nSee spaCy for an actual implementation.",
      factory.get()},
      doc2{"Another sample.", factory.get()},
      doc3{"All examples\nshould\nshare the model.", factory.get("default")};

  // Check whether they actually share the model
  //    1 of the pointers is in the factory;
  //    1 of the pointers is in the current get;
  cout << "The default model is currently in use by "
       << factory.get().use_count() - 2 << " clients." << "\n\n";

  // Run analysis
  for (Document *doc : {&doc1, &doc2, &doc3}) {
    doc->analyze();
    cout << "===================================\n" << doc->text() << "\n";
    cout << "===================================\n";
    for (int i = 0; i < doc->tags().size(); i++) {
      cout << "'" << doc->tokens()[i] << "': " << str(doc->tags()[i]) << "; ";
    }
    cout << "\n\n";
  }

  return 0;
}
