# How to use the Makefile
	# just type in this command line: make shop {arg1} {arg2} {arg3} {arg4}
	# Your code will compile or re-compile again then run 

# Erease the old compiled file
erase_old_file:
	rm -rf sleepingBarbers

# Compile file file
compile: erase_old_file
	g++ driver.cpp shop.cpp -o sleepingBarbers -lpthread

# Run the file
shop: compile
	./sleepingBarbers $(filter-out $@, $(MAKECMDGOALS))

# Run and check leak
valgrind_shop: compile
	valgrind ./sleepingBarbers $(filter-out $@, $(MAKECMDGOALS)) 