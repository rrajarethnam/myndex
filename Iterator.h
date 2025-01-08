#include "Page.h"
#include <queue>

template <typename K, typename V, typename P> class Iterator {
private:
    std::deque<P*> pages;
    typename std::deque<P*>::iterator pageIterator;
    int index;
    
public:
    Iterator(const std::deque<P*>& pages){
        this->pages = pages;
        this->pageIterator = this->pages.begin();
    }

    bool isEnd(){
        return this->pageIterator == pages.end();
    }

    Iterator& operator++(){
        if( this->pageIterator != pages.end() ){
            index++;
            if( index >= (*this->pageIterator)->count() ){
                this->pageIterator++;
                index = 0;
            }
        }
        return *this;
    }
    Iterator operator++(int){
        Iterator it = *this;
        if( this->pageIterator != pages.end() ){
            index++;
            if( index >= (*this->pageIterator)->count() ){
                this->pageIterator++;
                index = 0;
            }
        }
        return it;
    }

    V* operator*(){
        if( this->pageIterator != pages.end() )
            return (*this->pageIterator)->get(index);
        else
            return NULL;
    }
};