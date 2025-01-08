#include "Iterator.h"
#include "Page.h"
#include "FlatPage.h"

#include <queue>
#include <gtest/gtest.h>



TEST(Iterator, isEnd){
    std::deque<FlatPage<int, int>*> q;
    FlatPage<int, int>* p = new FlatPage<int, int>(10, true);
    for(int i=0; i<10; i++){
        p->add(i, i);
    }
    q.push_back(p);

    FlatPage<int, int>* p1 = new FlatPage<int, int>(10, true);
    for(int i=10; i<20; i++){
        p1->add(i, i);
    }

    q.push_back(p1);

    FlatPage<int, int>* p2 = new FlatPage<int, int>(10, true);
    for(int i=20; i<30; i++){
        p2->add(i, i);
    }

    q.push_back(p2);
    
    

    Iterator<int, int, FlatPage<int, int>> it(q);
    EXPECT_FALSE(it.isEnd());
    for(int i=0; i<30; i++){
        it++;
    }
    EXPECT_TRUE(it.isEnd());
}
