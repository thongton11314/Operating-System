clean:
	rm -rf *.o

shop: clean
	g++ driver.cpp shop.cpp -o sleepingBarbers -lpthread

111: shop
	./sleepingBarbers 1 1 1 1000

112: shop
	./sleepingBarbers 1 1 2 1000

113: shop
	./sleepingBarbers 1 1 3 1000

123: shop
	./sleepingBarbers 1 2 3 1000

211: shop
	./sleepingBarbers 2 1 1 1000

212: shop
	./sleepingBarbers 2 1 2 1000

1110: shop
	./sleepingBarbers 1 1 10 1000

3110: shop
	./sleepingBarbers 3 1 10 1000

213: shop
	./sleepingBarbers 2 1 3 1000
313: shop
	./sleepingBarbers 3 1 3 1000