obj = posix_sem.o
src = posix_sem_test.c

pc = producer_consumer_posix.o
src_pc = producer_consumer_posix_sem.c

test:
	gcc $(src) -o $(obj) -lpthread

pc_posix:
	gcc $(src_pc) -o $(pc) -lpthread

pc_posix_x86:
	gcc $(src_pc) -o $(pc) -lpthread -D x86

