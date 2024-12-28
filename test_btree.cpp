#include "btree.h"
#include <cassert>
#include <limits>
#include <iostream>

int main(int argc, char* argv[]){
	int begin = 1;
	int end = 7;
	Btree<int, int> btree(4, std::numeric_limits<int>::min(), std::numeric_limits<int>::min(), true);
	for(int i=begin; i<=end; i++){
		btree.put(i, i);
		//std::cout << "inserted:" << i << " and height:" << btree.get_height() << std::endl;
	}
	std::cout << "Insertion complete, height:" << btree.get_height() << std::endl;

//	char c;
//	std::cin >> c;
		
	for(int i=begin; i<=end; i++){
		assert(*btree.get(i) == i);
		//std::cout << "value:" << *btree.get(i) << std::endl;
	}

	std::cout << "Retrieval complete" << std::endl;

	for(int i=end; i>=begin; i--){
		assert(*btree.get(i) == i);
		btree.deleteKey(i);
		std::cout << "deleted:" << i << " and height:" << btree.get_height() << std::endl;
	}

	std::cout << "Deletion complete, height:" << btree.get_height() << std::endl;

		
	std::cout << "Test Passed" << std::endl;
}
