#pragma once

#include <map>
#include <fstream>
#include <cstring>
#include <cassert>
#include <iostream>

#include "Page.h"

template<class Key, class Value, class PageType> class Btree;

template<class Key, class Value> class TreePage : public Page<Key, Value>{
private:
    bool bottom;
    std::map<Key, TreePage*>* pages;
    std::map<Key, Value>* items;
    unsigned int order;

    //friend class Btree<Key, Value, PageType>;

public:
    TreePage(bool bottom, int order){
        this->bottom = bottom;
        this->order = order;
        if (bottom) {
            this->items = new std::map<Key, Value>();
        } else {
            this->pages = new std::map<Key, TreePage*>();
        }
    }
    //void close();

    void print(){
        if (this->bottom) {
            std::cout << "{";
            for(typename std::map<Key, Value>::iterator it = this->items->begin(); it != this->items->end(); it++){
                std::cout << it->first << ", ";
            }
            std::cout << "}" << std::endl;
        } else {
            std::cout << "[";
            for(typename std::map<Key, TreePage*>::iterator it = this->pages->begin(); it != this->pages->end(); it++){
                std::cout << it->first << ", ";
            }
            std::cout << "]" << std::endl;
        }        
    }

    void printKeys(){
        if (this->bottom) {
            std::cout << "{";
            for(typename std::map<Key, Value>::iterator it = this->items->begin(); it != this->items->end(); it++){
                std::cout << it->first << ", ";
            }
            std::cout << "}";
        } else {
            std::cout << "[";
            for(typename std::map<Key, TreePage*>::iterator it = this->pages->begin(); it != this->pages->end(); it++){
                std::cout << it->first << ", ";
                it->second->printKeys();
            }
            std::cout << "]";
        }
    }

    void save(const std::string& filename){

    }

    void open(const std::string& filename){

    }

    Value* getValue(Key key){
        assert(this->isExternal());
        if (this->items->find(key) != this->items->end()) {
            Value* value = &(*this->items)[key];
            return value;
        } else {
            return NULL;
        }
    }

    Key firstKey(){
        if (this->bottom) {
            return this->items->begin()->first;
        } else {
            return this->pages->begin()->first;
        }
    }

    Key secondKey() {
        if (this->bottom) {
            return (++this->items->begin())->first;
        } else {
            return (++this->pages->begin())->first;
        }
    }

    Page<Key, Value>* firstPage(){
        if (this->bottom) {
            return NULL;
        } else {
            return this->pages->begin()->second;
        }
    }

    Page<Key, Value>* lastPage() {
        if (this->bottom) {
            return NULL;
        } else {
            return this->pages->rbegin()->second;
        }
    }

    void add(Key key, Value value) {
        assert(this->isExternal());
        // add key to page
        (*this->items)[key] = value;
    }

    void add(Key key, Page<Key, Value>* page) {
        assert(!this->isExternal());
        // add page to page
        (*this->pages)[key] = (TreePage*)page;
    }

    bool isExternal() {
        return this->bottom;
    }

    bool contains(Key key){
        if (this->bottom) {
            return this->items->find(key) != this->items->end();
        } else {
            return this->pages->find(key) != this->pages->end();
        }
    }

    TreePage* findTreePage(Key key) {
        typename std::map<Key, TreePage*>::iterator it = this->pages->lower_bound(key);
        if (it == this->pages->begin() && it->first != key) {
            // Key is less than the smallest key in the map
            return nullptr; // or handle this case as needed
        }
        if (it == this->pages->end() || it->first != key) {
            // Key is greater than the largest key or not found
            --it;
        }
        return it->second;
    }

    TreePage* next(Key key) {
        if (this->bottom) {
            return NULL;
        }
        if (this->pages->find(key) != this->pages->end()) {
            return (*this->pages)[key];
        }
        return findTreePage(key);
    }

    bool isFull() {
        if (this->bottom) {
            return this->items->size() >= this->order;
        } else {
            return this->pages->size() >= this->order;
        }
    }

    TreePage* split(){
        TreePage* page = new TreePage(this->bottom, this->order);
        if( this->bottom) {
            int half = this->items->size() / 2;
            typename std::map<Key, Value>::iterator it = this->items->begin();
            std::advance(it, half);
            while (it != this->items->end()) {
                page->add(it->first, it->second);
                it = this->items->erase(it);
            }
        } else {
            int half = this->pages->size() / 2;
            typename std::map<Key, TreePage*>::iterator it = this->pages->begin();
            std::advance(it, half);
            while (it != this->pages->end()) {
                page->add(it->first, it->second);
                it = this->pages->erase(it);
            }
        }
        return page;
    }

    TreePage* nextPageOf(Page<Key, Value>* page) {
        assert(!this->bottom);
        typename std::map<Key, TreePage*>::iterator it = ++this->pages->find(page->firstKey());
        if(it != this->pages->end()){
            return it->second;
        } else {
            return NULL;
        }
    }

    TreePage* prevPageOf(Page<Key, Value>* page) {    
        assert(!this->bottom);
        typename std::map<Key, TreePage*>::iterator it = this->pages->find(page->firstKey());
        if(it != this->pages->begin()){
            return (--it)->second;
        } else {
            return NULL;
        }
    }

    unsigned int count(){
        if (this->bottom) {
            return this->items->size();
        } else {
            return this->pages->size();
        }
    }

    void detach(){
        if (this->bottom) {
            this->items = NULL;
        } else {
            this->pages = NULL;
        }
    }

    Page<Key, Value>* merge(Page<Key, Value>* page) {
        TreePage* treePage = (TreePage*)page;
        if (this->bottom) {
            for(typename std::map<Key, Value>::iterator it = treePage->items->begin(); it != treePage->items->end(); it++){
                this->items->insert(*it);
            }
        } else {
            for(typename std::map<Key, TreePage*>::iterator it = treePage->pages->begin(); it != treePage->pages->end(); it++){
                this->pages->insert(*it);
            }

        }

        treePage->detach();

        return this;
    }

    void remove(Key key){
        if (this->bottom) {
            this->items->erase(key);
        } else {
            TreePage* page = this->pages->operator[](key);
            if(page != NULL){
                this->pages->erase(key);
            }
        }
    }

    void replaceKey(Key oldKey, Key newKey){
        if (this->bottom) {
            Value value = this->items->operator[](oldKey);
            this->items->erase(oldKey);
            this->items->insert(std::pair<Key, Value>(newKey, value));
        } else {
            TreePage* page = this->pages->operator[](oldKey);
            if(page != NULL){
                this->pages->erase(oldKey);
                this->pages->insert(std::pair<Key, TreePage*>(newKey, page));
            }
        }
    }


    ~TreePage(){
        if (this->bottom) {
            if(this->items != NULL)
                delete this->items;
        } else {
            if(this->pages != NULL) {
                for(typename std::map<Key, TreePage*>::iterator it = this->pages->begin(); it != this->pages->end(); it++){
                    delete it->second;
                }
                delete this->pages;
            }
        }
    }
};
