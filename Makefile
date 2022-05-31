all : 
	gcc -o knocking knocking_service.c -lnetfilter_queue

clean:
	rm knocking
