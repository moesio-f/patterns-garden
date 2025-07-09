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

python-factory-method:
	@echo "[Makefile] Running Python Factory Method Pattern sample..."
	@python ./python/creational/factory_method.py

python-prototype:
	@echo "[Makefile] Running Python Prototype Pattern sample..."
	@python ./python/creational/prototype.py

python-object-pool:
	@echo "[Makefile] Running Python Object Pool Pattern sample..."
	@python ./python/creational/object_pool.py

python-adapter:
	@echo "[Makefile] Running Python Adapter Pattern sample..."
	@python ./python/structural/adapter.py

c-abstract-factory:
	@echo "[Makefile] Running C Abstract Factory Pattern sample..."
	@$(call run_c,"./c/creational/abstract_factory.c")

c-builder:
	@echo "[Makefile] Running C Builder Pattern sample..."
	@$(call run_c,"./c/creational/builder.c")

cpp-abstract-factory:
	@echo "[Makefile] Running C++ Abstract Factory Pattern sample..."
	@$(call run_cpp,"./cpp/creational/abstract_factory.cpp")

cpp-builder:
	@echo "[Makefile] Running C++ Builder Pattern sample..."
	@$(call run_cpp,"./cpp/creational/builder.cpp")

cpp-factory-method:
	@echo "[Makefile] Running C++ Factory Method Pattern sample..."
	@$(call run_cpp,"./cpp/creational/factory_method.cpp")

cpp-prototype:
	@echo "[Makefile] Running C++ Prototype Pattern sample..."
	@$(call run_cpp,"./cpp/creational/prototype.cpp")

cpp-object-pool:
	@echo "[Makefile] Running C++ Object Pool Pattern sample..."
	@$(call run_cpp,"./cpp/creational/object_pool.cpp")

cpp-adapter:
	@echo "[Makefile] Running C++ Adapter Pattern sample..."
	@$(call run_cpp,"./cpp/structural/adapter.cpp")

cpp-bridge:
	@echo "[Makefile] Running C++ Bridge Pattern sample..."
	@$(call run_cpp,"./cpp/structural/bridge.cpp")

java-abstract-factory:
	@echo "[Makefile] Running Java Abstract Factory Pattern sample..."
	@$(call run_java,"./java/creational/AbstractFactory.java")
