# How to use the Makefile
	# just type in this command line: make shop {arg1} {arg2} {arg3} {arg4}
	# Your code will compile or re-compile again then run 

transfer_lab:
	scp shop.h shop.cpp driver.cpp Makefile tonthong@csslab5.uwb.edu:/home/NETID/tonthong/program4;

access_lab:
	ssh tonthong@csslab5.uwb.edu

remove:
	rm -rf driver.cpp shop.cpp shop.h

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

# 1barber_1chair_10customer_1000stime
1_1_1_1000: compile
	./sleepingBarbers 1 1 1 1000

# 3barber_1chair_10customer_1000stime
3_1_10_1000: compile
	./sleepingBarbers 3 1 10 1000

