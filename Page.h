#pragma once
#include <string>

//Interface for Btree pages
template<class Key, class Value> class Page{
public:
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

    virtual void add(Key key, Value value) = 0;
    virtual void add(Key key, Page* page) = 0;
    virtual bool isExternal() = 0;
    virtual Page* next(Key key) = 0;
    virtual bool isFull() = 0;
    virtual Page* split() = 0;
    virtual Page* merge(Page* page) = 0;
    virtual void remove(Key key) = 0;
    virtual void replaceKey(Key oldKey, Key newKey) = 0;
    virtual void printKeys() = 0;
    virtual void print() = 0;
    virtual unsigned int count() = 0;
    virtual void detach() = 0;

    virtual void save(const std::string& filename) = 0;
    virtual void open(const std::string& filename) = 0;
    virtual ~Page() {};
};

