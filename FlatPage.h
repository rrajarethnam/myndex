#pragma once
#include <map>
#include <fstream>
#include <cstring>
#include <cassert>
#include <iostream>


#include "Page.h"

template<class Key, class Value> class FlatPage : public Page<Key, Value>{
protected:
    Key* keys;
    Value* values;
    Page<Key, Value>** pages;

    unsigned int size;
    unsigned int order;
    bool bottom;
    bool *loaded;
    //friend class Btree<Key, Value, PageType>;
public:

    FlatPage(){
        this->size = 0;
        this->keys = NULL;
        this->values = NULL;
        this->pages = NULL;
        this->loaded = NULL;
    }
    FlatPage(const std::string& filename, int order){
        this->order = order;
        this->open(filename);
    }

    FlatPage(int order, bool bottom){
        this->order = order;
        this->bottom = bottom;
        this->size = 0;
        this->keys = new Key[2*order];
        if(!bottom){
            this->pages = new Page<Key, Value>*[2*order];
        } else {
            this->values = new Value[2*order];
        }
    }

    void save(const std::string& filename){
        std::ofstream file;
        if(this->bottom){
            file.open(filename + ".values.idx");
        } else {
            file.open(filename + ".idx");
        }
        file.write((char*)&this->size, sizeof(this->size));
        //copy using memcopy
        file.write((char*)this->keys, this->size * sizeof(Key));
        if(this->bottom){
            file.write((char*)this->values, this->size * sizeof(Value));
        } else {
            for(int i=0; i<this->size; i++){
                this->pages[i]->save(filename + "_" + std::to_string(i));
            }
        }
    }

    virtual void open(const std::string& filename){
        this->bottom = false;
        std::ifstream file;
        file.open(filename + ".idx");
        if(!file.is_open()){
            file.close();
            file.open(filename + ".values.idx");
            this->bottom = true;
            if(!file.is_open()){
                std::cout << "Error: File not found" << std::endl;
                assert(false);
            }
        }

        file.read((char*)&this->size, sizeof(this->size));

        //copy using memcopy
        this->keys = new Key[2*this->order];
        file.read((char*)this->keys, this->size * sizeof(Key));
        
        // Determine the actual size of the array based on bytes read
        //std::streamsize bytesRead = file.gcount();
        //this->size = bytesRead / sizeof(Key);
        
        if(this->bottom){
            this->values = new Value[2*this->order];
            file.read((char*)this->values, this->size * sizeof(Value));
        } else {
            this->pages = new Page<Key, Value>*[2*this->order];
            for(int i=0; i<this->size; i++){
                this->pages[i] = new FlatPage(filename + "_" + std::to_string(i), this->order);
            }
        }
    }

    void print(){
        if(this->bottom){
            std::cout << "{";
            for(int i=0; i<this->size; i++){
                std::cout << this->keys[i] << "=>" << this->values[i] << ", ";
                std::cout.flush();
            }
            std::cout << "}";
        } else {
            std::cout << "[";
            for(int i=0; i<this->size; i++){
                std::cout << this->keys[i]  << ", ";
                std::cout.flush();
                this->pages[i]->print();
            }
            std::cout << "]";
        }
        std::cout.flush();
    }

    void printKeys(){
        if(this->bottom){
            std::cout << "{";
            for(int i=0; i<this->size; i++){
                std::cout << this->keys[i] << "=>" << this->values[i] << ", ";
            }
            std::cout << "}";
        } else {
            std::cout << "[";
            for(int i=0; i<this->size; i++){
                std::cout << this->keys[i]  << ", ";
            }
            std::cout << "]";
        }
        std::cout << std::endl;
    }

    bool isExternal(){
        return this->bottom;
    }

    Value* getValue(Key key){
        assert(this->isExternal());
        int first = 0;
        int last = this->size - 1;
        while(first <= last){
            int mid = first + (last - first) / 2;
            if (this->keys[mid] == key) {
                return &this->values[mid];
            }
            if (this->keys[mid] < key) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }
        return NULL;
    }

    int getIndexOf(Key key){
        int first = 0;
        int last = this->size - 1;
        int mid = first;
        while(first <= last){
            mid = first + (last - first) / 2;
            if (this->keys[mid] == key) {
                return mid;
            }
            if (this->keys[mid] < key) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }
        return mid;
    }

    Value* getValueAt(int index){
        assert(this->isExternal());
        return &this->values[index];
    }

    Key getKeyAt(int index){
        return this->keys[index];
    }

    Page<Key, Value>* getPageAt(int index){
        assert(!this->isExternal());
        return this->pages[index];
    }

    void add(Key key, Value value){
        assert(this->isExternal());
        //Find the index of the key
        int first = 0;
        int last = this->size - 1;
        while(first <= last){
            int mid = first + (last - first) / 2;
            if (this->keys[mid] == key) {
                this->values[mid] = value;
                return;
            }
            if (this->keys[mid] < key) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }
        //Insert the key and value
        //Shift the keys and values to the right using memcopy
        memmove(this->keys + first + 1, this->keys + first, (this->size - first) * sizeof(Key));
        memmove(this->values + first + 1, this->values + first, (this->size - first) * sizeof(Value));

        this->keys[first] = key;
        this->values[first] = value;
        this->size++;

    }

    void add(Key key, Page<Key, Value>* page){
        assert(!this->isExternal());
        //Find the index of the key
        int first = 0;
        int last = this->size - 1;
        while(first <= last){
            int mid = first + (last - first) / 2;
            if (this->keys[mid] == key) {
                this->pages[mid] = page;
                return;
            }
            if (this->keys[mid] < key) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }
        //Insert the key and value
        //Shift the keys and values to the right using memcopy
        memmove(this->keys + first + 1, this->keys + first, (this->size - first) * sizeof(Key));
        memmove(this->pages + first + 1, this->pages + first, (this->size - first) * sizeof(FlatPage*));
        this->keys[first] = key;
        this->pages[first] = page;
        this->size++;
    }

    Page<Key, Value>* next(Key key){
        if (this->bottom) {
            return NULL;
        }
        //Find the index of the key
        int first = 0;
        int last = this->size - 1;
        while(first <= last){
            int mid = first + (last - first) / 2;
            if (this->keys[mid] == key) {
                return this->pages[mid];
            }
            if (this->keys[mid] < key) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }
        if (first > last) {
            first--;
        }
        return this->pages[first];
    }

    FlatPage* split(){
        FlatPage* page = new FlatPage(this->order, this->bottom);
        int half = this->size / 2 + (this->size % 2);
        int otherHalf = this->size - half;
        memcpy(page->keys, this->keys + half, otherHalf * sizeof(Key));
        this->size = half;
        page->size = otherHalf;
        if( this->bottom) {
            memcpy(page->values, this->values + half, otherHalf * sizeof(Value));
        } else {
            memcpy(page->pages, this->pages + half, half * sizeof(FlatPage*));
        }

        return page;
    }

    Page<Key, Value>* nextPageOf(Page<Key, Value>* page) {
        assert(!this->bottom);
        Key key = page->firstKey();
        int first = 0;
        int last = this->size - 1;
        while(first <= last){
            int mid = first + (last - first) / 2;
            if (this->keys[mid] == key) {
                if(mid + 1 < this->size)
                return this->pages[mid+1];
            }
            if (this->keys[mid] < key) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }
        return NULL;
    }

    Page<Key, Value>* prevPageOf(Page<Key, Value>* page) {
        assert(!this->bottom);
        Key key = page->firstKey();
        int first = 0;
        int last = this->size - 1;
        while(first <= last){
            int mid = first + (last - first) / 2;
            if (this->keys[mid] == key) {
                if(mid - 1 >= 0)
                return this->pages[mid-1];
            }
            if (this->keys[mid] < key) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }
        return NULL;
    }

    unsigned int count(){
        return this->size;
    }

    Page<Key, Value>* merge(Page<Key, Value>* page) {
        FlatPage* flatPage = (FlatPage*)page;
        if (this->bottom) {
            memcpy(this->keys + this->size, flatPage->keys, flatPage->size * sizeof(Key));
            memcpy(this->values + this->size, flatPage->values, flatPage->size * sizeof(Value));
            this->size += flatPage->size;
        } else {
            memcpy(this->keys + this->size, flatPage->keys, flatPage->size * sizeof(Key));
            memcpy(this->pages + this->size, flatPage->pages, flatPage->size * sizeof(FlatPage*));
            this->size += flatPage->size;
        }

        flatPage->detach();
        
        return this;
    }

    void remove(Key key){
        if (this->bottom) {
            int first = 0;
            int last = this->size - 1;
            while(first <= last){
                int mid = first + (last - first) / 2;
                if (this->keys[mid] == key) {
                    memmove(this->keys + mid, this->keys + mid + 1, (this->size - mid - 1) * sizeof(Key));
                    memmove(this->values + mid, this->values + mid + 1, (this->size - mid - 1) * sizeof(Value));
                    this->size--;
                    return;
                }
                if (this->keys[mid] < key) {
                    first = mid + 1;
                } else {
                    last = mid - 1;
                }
            }
        } else {
            int first = 0;
            int last = this->size - 1;
            while(first <= last){
                int mid = first + (last - first) / 2;
                if (this->keys[mid] == key) {
                    //erase
                    memmove(this->keys + mid, this->keys + mid + 1, (this->size - mid - 1) * sizeof(Key));
                    memmove(this->pages + mid, this->pages + mid + 1, (this->size - mid - 1) * sizeof(FlatPage*));
                    this->size--;
                    return;
                }
                if (this->keys[mid] < key) {
                    first = mid + 1;
                } else {
                    last = mid - 1;
                }
            }
        }
    }

    void replaceKey(Key oldKey, Key newKey){
        int first = 0;
        int last = this->size - 1;
        while(first <= last){
            int mid = first + (last - first) / 2;
            if (this->keys[mid] == oldKey) {
                this->keys[mid] = newKey;
                return;
            }
            if (this->keys[mid] < oldKey) {
                first = mid + 1;
            } else {
                last = mid - 1;
            }
        }

    }


    Key firstKey(){
        return this->keys[0];
    }

    Key secondKey(){
        return this->keys[1];
    }

    Page<Key, Value>* firstPage(){
        return this->pages[0];
    }

    Page<Key, Value>* lastPage(){
        return this->pages[this->size - 1];
    }

    bool isFull(){
        return this->size >= this->order;
    }

    void detach(){
        if (this->bottom) {
            this->values = NULL;
            this->keys = NULL;
        } else {
            this->pages = NULL;
        }
    }

    ~FlatPage(){
        delete[] this->keys;
        if(this->bottom){
            delete[] this->values;
        } else {
            if(this->pages != NULL)
                for(int i=0; i<this->size; i++){
                    delete this->pages[i];
                }
            delete[] this->pages;
        }
    }

};
