#pragma once
#include <string>
#include <iostream>
#include <fstream>

//Interface for Btree pages
template<class Key, class Value> class Page{
public:
    virtual std::string getId() const = 0;
    virtual Value* getValue(Key key) = 0;
    virtual int getIndexOf(Key key) = 0;
    virtual Value* getValueAt(int index) = 0;
    virtual Key getKeyAt(int index) = 0;
    virtual Page* getPageAt(int index) = 0;
    virtual Key firstKey() = 0;
    virtual Key lastKey() = 0;
    virtual Key secondKey() = 0;
    virtual Page* firstPage() = 0;
    virtual Page* lastPage() = 0;
    virtual Page* nextPageOf(Page* page) = 0;
    virtual Page* prevPageOf(Page* page) = 0;
    virtual bool isExternal() = 0;
    virtual bool isFull() = 0;
    virtual Page* next(Key key) = 0;
    virtual void printKeys() = 0;
    virtual void print() = 0;
    virtual void draw(std::ofstream &file) {
        this->open();
        file << "node" << this << " [shape=record, label=\"{";
        for(int i=0; i<this->count(); i++){
            file << "<f" << i << "> " << this->getKeyAt(i);
            if(i < this->count() - 1){
                file << " | ";
            }
        }
        file << "}\"];" << std::endl;
        
        if (!this->isExternal()) {
            for(int i=0; i<this->count(); i++){
                file << "node" << this << ":f" << i << " -> node" << this->getPageAt(i) << ";" << std::endl;
                this->getPageAt(i)->draw(file);
            }
        }
    }

    virtual unsigned int count() = 0;

    virtual void add(Key key, Value value) = 0;
    virtual void add(Key key, Page* page) = 0;
    virtual Page* split() = 0;
    virtual Page* merge(Page* page) = 0;
    virtual void remove(Key key) = 0;
    virtual void replaceKey(Key oldKey, Key newKey) = 0;
    virtual void detach() = 0;

    virtual void save() = 0;
    virtual void open(bool reload=false) = 0;
    virtual ~Page() {};
};

#define SAVE if(this->filename != "") this->save(this->filename);

