all: paging_sim

clean: 
	rm -f paging_sim

paging_sim: paging_sim.c
	gcc -Wall -o paging_sim paging_sim.c

