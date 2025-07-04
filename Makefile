# Utilities
define run_c
    $(eval $@_FNAME = $(1))
    gcc -lm "${$@_FNAME}"
	./a.out
	rm ./a.out
endef

define run_cpp
    $(eval $@_FNAME = $(1))
    g++ "${$@_FNAME}"
	./a.out
	rm ./a.out
endef

define run_java
    $(eval $@_FNAME = $(1))
    javac "${$@_FNAME}" -d .
	java Main
	rm *.class
endef


default: 
	@echo "[Makefile] Run samples using {python,c,cpp,java}-{design_pattern_name}"
	@rm -f *.out
	@rm -f *.class

python-abstract-factory:
	@echo "[Makefile] Running Python Abstract Factory Pattern sample..."
	@python ./python/creational/abstract_factory.py

python-builder:
	@echo "[Makefile] Running Python Builder Pattern sample..."
	@python ./python/creational/builder.py

c-abstract-factory:
	@echo "[Makefile] Running C Abstract Factory Pattern sample..."
	@$(call run_c,"./c/creational/abstract_factory.c")

cpp-abstract-factory:
	@echo "[Makefile] Running C++ Abstract Factory Pattern sample..."
	@$(call run_cpp,"./cpp/creational/abstract_factory.cpp")

java-abstract-factory:
	@echo "[Makefile] Running Java Abstract Factory Pattern sample..."
	@$(call run_java,"./java/creational/AbstractFactory.java")
