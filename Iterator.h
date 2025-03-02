#pragma once
#include "Page.h"
#include <vector>
#include <iostream>

template <typename Key, typename Value, typename PageType> class Iterator {
private:
    std::vector<PageType*> pages;
    Key from;
    Key to;
    typename std::vector<PageType*>::iterator pageIterator;
    int index;
    
public:
    Iterator(const std::vector<PageType*>& pages, Key from, Key to){
        this->pages = pages;
        this->pageIterator = this->pages.begin();
        this->from = from;
        this->to = to; 
        this->index = pages.front()->getIndexOf(from);
    }

    bool isEnd(){
        Page<Key, Value>* page = *this->pageIterator;
        return this->pageIterator == pages.end() || (*this->pageIterator == *pages.rbegin() && index > page->getIndexOf(to));
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

        if(*this->pageIterator == *pages.rbegin()){
            Page<Key, Value>* page = *this->pageIterator;
            if(index <= page->getIndexOf(to) && index < page->count())
                index++;
            return it;
        }
        if( this->pageIterator != pages.end() ){
            index++;
            if( index >= (*this->pageIterator)->count() ){
                this->pageIterator++;
                index = 0;
            }
        }
        return it;
    }

    Value* operator*(){
        if( this->pageIterator != pages.end() )
            return (*this->pageIterator)->getValueAt(index);
        else
            return NULL;
    }
};