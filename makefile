scheduler: mmu.cpp
	g++ -g -w mmu.cpp -o mmu # I always compile with -g to enable debugging
clean:
	rm -f mmu *~

