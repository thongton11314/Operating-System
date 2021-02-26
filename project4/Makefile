erase_old_file:
	rm -rf sleepingBarbers

compile: erase_old_file
	g++ driver.cpp shop.cpp -o sleepingBarbers -lpthread

shop: compile
	./sleepingBarbers $(filter-out $@, $(MAKECMDGOALS))
	