default: ;

python-abstract-factory:
	@echo "[Makefile] Running Python Abstract Factory Pattern sample..."
	@python ./python/creational/abstract_factory.py

c-abstract-factory:
	@echo "[Makefile] Running C Abstract Factory Pattern sample..."
	@gcc -lm ./c/creational/abstract_factory.c
	@./a.out
	@rm ./a.out
