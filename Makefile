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

default: 
	@echo "[Makefile] Run samples using {python,c,cpp,java}-{design_pattern_name}"
	@rm -f *.out

python-abstract-factory:
	@echo "[Makefile] Running Python Abstract Factory Pattern sample..."
	@python ./python/creational/abstract_factory.py

c-abstract-factory:
	@echo "[Makefile] Running C Abstract Factory Pattern sample..."
	@$(call run_c,"./c/creational/abstract_factory.c")

cpp-abstract-factory:
	@echo "[Makefile] Running C++ Abstract Factory Pattern sample..."
	@$(call run_cpp,"./cpp/creational/abstract_factory.cpp")
