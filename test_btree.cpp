#include "btree.h"
#include <cassert>
#include <limits>
#include <iostream>

int main(int argc, char* argv[]){
	int begin = 1;
	int end = 100;

	if(argc > 1)
		end = atoi(argv[1]);



	std::cout << "Random insertions and deletions test" << std::endl;
	int* randoms = new int[end];
	Btree<int, int> btree(4000, std::numeric_limits<int>::min(), std::numeric_limits<int>::min());

	for(int i=0; i<end; i++){
		do {
			randoms[i] = rand() % end;
			//std::cout << randoms[i] << std::endl;
		} while(btree.get(randoms[i]) != NULL);

		//std::cout << "Inserting:" << randoms[i] << " at" << i << std::endl;
		btree.put(randoms[i], randoms[i]);
	}

	std::cout<< "test input generated" << std::endl;
	std::cout << "Height:" << btree.get_height() << std::endl;
	btree.save("btree");
	Btree<int, int> btree1("btree");
	//btree.print();
	std::cout << "Height:" << btree1.get_height() << std::endl;
	//btree1.print();
	std::cout << "Saved" << std::endl;
	std::cout << "Testing get and delete" << std::endl;
	//btree.print();
	for(int i=(end-1); i>=0; i--){
		assert(*btree1.get(randoms[i]) == randoms[i]);
		btree1.deleteKey(randoms[i]);
		//std::cout << "Deleted :" << randoms[i] << std::endl;
		//btree.print();
	}
	
	std::cout << "Deletions complete!"<< std::endl;

	std::cout << "Height:" << btree.get_height() << std::endl;
	std::cout << "Test Passed" << std::endl;

}
