#include <map>
#include <fstream>
#include <cstring>
#include <cassert>
#include <iostream>

#include "Page.h"
#include "TreePage.h"
#include "FlatPage.h"
#include "Iterator.h"



template<class Key, class Value, class PageType> class Btree{
private:
    Page<Key, Value>* root;
    int order;  // max children per B-tree node = order-1
    int height; // height of the B-tree
    int n;      // number of key-value pairs in the B-tree
    bool memoryOnly;

public:
    Btree(int order, Key sentinel, Value sentinelValue, bool memoryOnly = false){
        this->memoryOnly = memoryOnly;
        this->order = order;
        this->root = new PageType(this->order, true);
        this->root->add(sentinel, sentinelValue);
        this->height = 1;
    }

    Btree(std::string name){
        this->memoryOnly = false;
        std::ifstream file;
        file.open(name + ".idx.meta");
        file >> this->order;
        file >> this->height;
        file >> this->n;
        file.close();

        this->root = new PageType(this->order, true);
        this->root->open(name);
    }

    Value* get(Key key){
        return this->get(this->root, key);
    }

    void get(Page<Key, Value>* page, Key from, Key to, std::vector<Page<Key, Value>*>& pages){
        int fromIndex = page->getIndexOf(from);
        int toIndex = page->getIndexOf(to);
        if(!page->isExternal()){
            int begin = 0;
            if(fromIndex >= 0 && fromIndex < page->count()){
                begin = fromIndex;
            }
            int end = page->count()-1;
            if(toIndex >= 0 && toIndex < page->count()){
                end = toIndex;
            }
            for(int i=begin; i<=end; i++){
                this->get(page->getPageAt(i), from, to, pages);
            }
        } else {
            if((fromIndex >= 0 && fromIndex < page->count())
            || (fromIndex < 0 && toIndex >= page->count()) 
            || (toIndex >= 0 && toIndex < page->count())){
                pages.push_back(page);
            }            
        }
    }
    
    Iterator<Key, Value, Page<Key, Value>> get(Key from, Key to){
        std::vector<Page<Key, Value>*> q;
        this->get(this->root, from, to, q);
        return Iterator<Key, Value, Page<Key, Value>>(q, from, to);
    }

    Value* get(Page<Key, Value>* page, Key key){
        if (page->isExternal()) {
            return page->getValue(key);
        }
        Page<Key, Value>* next = page->next(key);
        return this->get(next, key);
    }

    void put(Key key, Value value){
        this->put(this->root, key, value);
        this->n++;
        if (this->root->isFull()) {
            Page<Key, Value>* left = this->root;
            Page<Key, Value>* right = this->root->split();
            this->root = new PageType(this->order, false);
            this->root->add(left->firstKey(), left);
            this->root->add(right->firstKey(), right);
            this->height++;
        }
    }

    void deleteKey(Key key){        
        this->deleteKey(this->root, key);
        this->n--;

        if(this->root->count() == 1 && this->height > 1){
            Page<Key, Value>* oldRoot = this->root;
            this->root = this->root->firstPage();
            oldRoot->detach();
            delete oldRoot;
            this->height--;
        }
    }

    void put(Page<Key, Value>* page, Key key, Value value){
        if (page->isExternal()) {
            page->add(key, value);
            return;
        }
        Page<Key, Value>* next = page->next(key);
        this->put(next, key, value);
        if (next->isFull()) {
            Page<Key, Value>* split_page = next->split();
            page->add(split_page->firstKey(), split_page);
        }
    }

    void deleteKey(Page<Key, Value>* page, Key key){
        if (page->isExternal()) {
            page->remove(key);
            return;
        }

        Page<Key, Value>* next = page->next(key);
        Key nextPageKey = next->firstKey();

        this->deleteKey(next, key);
        if(key == nextPageKey){
            page->replaceKey(key, next->firstKey());
            nextPageKey = next->firstKey();
        }
        if (next->count() < this->order/2){
            //find the previous page
            Page<Key, Value>* prev = page->prevPageOf(next);

            if(prev != NULL){
                page->remove(nextPageKey);
                prev->merge(next);
                delete next;

                if (prev->isFull()) {
                    Page<Key, Value>* split_page = prev->split();
                    page->add(split_page->firstKey(), split_page);
                }
            } else {
                //find the next page
                Page<Key, Value>* next_next = page->nextPageOf(next);
                if (next_next != NULL){
                    page->remove(next_next->firstKey());
                    next->merge(next_next);
                    delete next_next;
                    
                    if (next->isFull()) {
                        Page<Key, Value>* split_page = next->split();
                        page->add(split_page->firstKey(), split_page);
                    }
                } else {
                    std::cout << "Error: No previous or next page found" << std::endl;
                }
            }
        }
    }

    void save(std::string name){
        std::ofstream file;
        file.open(name + ".idx.meta");
        file << this->order << std::endl;
        file << this->height << std::endl;
        file << this->n << std::endl;
        file.close();

        this->root->save(name);
    }

    unsigned int count(){
        return this->n;
    }

    unsigned int get_height(){
        return this->height;
    }

    virtual void print(){
        this->root->print();
        std::cout << std::endl;
    }


    ~Btree(){
        //Delete the root page
        delete this->root;
    }
};

