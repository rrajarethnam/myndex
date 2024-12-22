#include "btree.h"
#include <cassert>
#include <limits>
#include <iostream>

int main(int argc, char* argv[]){
	Btree<int, int> btree(20, std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
	for(int i=-1000; i<10000; i++)
		btree.put(i, i);
		
	for(int i=-1000; i<10000; i++){
		assert(*btree.get(i) == i);
		std::cout << "value:" << *btree.get(i) << std::endl;
	}
		
	std::cout << "Test Passed" << std::endl;
}
