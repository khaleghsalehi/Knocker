all : 
	gcc -o knocking src/knocking_service.c -lnetfilter_queue

clean:
	rm knocking
