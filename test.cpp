#include "btree.h"
#include "CompoundObjectsFlatPage.h"
#include <cassert>
#include <limits>
#include <iostream>

std::string get(Btree<std::string, std::string, CompoundObjectsFlatPage<std::string, std::string>>& btree, char* key){
	std::string keystr = std::string(key);
	return *btree.get(keystr);
}

int main(int argc, char* argv[]){
	int begin = 1;
	int end = 10000;

	if(argc > 1)
		end = atoi(argv[1]);



	std::cout << "Random insertions and deletions test" << std::endl;
	int* randoms = new int[end];
	Btree<std::string, std::string, CompoundObjectsFlatPage<std::string, std::string>> btree(4000, std::string(), std::string());

	for(int i=0; i<end; i++){
		do {
			randoms[i] = rand() % end;
			//convert randoms[i] to string
			//std::cout << randoms[i] << std::endl;
		} while(btree.get(std::to_string(randoms[i])) != NULL);

		//std::cout << "Inserting:" << randoms[i] << " at" << i << std::endl;
		std::string random = std::to_string(randoms[i]);
		btree.put(random, random);
	}

	std::cout<< "test input generated" << std::endl;
	std::cout << "Height:" << btree.get_height() << std::endl;
	btree.save("btree");
	Btree<std::string, std::string, CompoundObjectsFlatPage<std::string, std::string>> btree1("btree");
	//btree.print();
	std::cout << "Height:" << btree1.get_height() << std::endl;
	//btree1.print();
	std::cout << "Saved" << std::endl;
	std::cout << "Testing get and delete" << std::endl;
	//btree.print();
	for(int i=(end-1); i>=0; i--){
		assert(*btree1.get(std::to_string(randoms[i])) == std::to_string(randoms[i]));
		btree1.deleteKey(std::to_string(randoms[i]));
		//std::cout << "Deleted :" << randoms[i] << std::endl;
		//btree1.print();
	}
	
	std::cout << "Deletions complete!"<< std::endl;

	std::cout << "Height:" << btree1.get_height() << std::endl;

	Iterator<std::string, std::string, Page<std::string, std::string>> it = btree.get("100", "200");	
	while(!it.isEnd()){
		std::cout << **it << std::endl;
		it++;
	}

	std::cout << "Test Passed" << std::endl;

}
