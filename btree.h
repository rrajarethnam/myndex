#include <map>
#include <fstream>
#include <cstring>
#include <cassert>
#include <iostream>

template<class Key, class Value> class Btree;

//Interface for Btree pages
template<class Key, class Value> class Page{
public:
    virtual Value* getValue(Key key) = 0;
    virtual Key firstKey() = 0;
    virtual Page* firstPage() = 0;
    virtual Page* lastPage() = 0;
    virtual void add(Key key, Value value) = 0;
    virtual void add(Key key, Page* page) = 0;
    virtual bool isExternal() = 0;
    virtual Page* next(Key key) = 0;
    virtual bool isFull() = 0;
    virtual Page* split() = 0;
    virtual Page* nextPage() = 0;
    virtual Page* prevPage() = 0;
    virtual Page* merge(Page* page) = 0;
    virtual void remove(Key key) = 0;
    virtual void printKeys() = 0;
    virtual unsigned int count() = 0;
    virtual void detach() = 0;
    virtual ~Page() {};
};

template<class Key, class Value> class TreePage : public Page<Key, Value>{
private:
    bool bottom;
    std::map<Key, TreePage*>* pages;
    std::map<Key, Value>* items;
    unsigned int order;
    TreePage* _nextPage;
    TreePage* _prevPage;

    friend class Btree<Key, Value>;

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

    void printKeys(){
        if (this->bottom) {
            for(typename std::map<Key, Value>::iterator it = this->items->begin(); it != this->items->end(); it++){
                std::cout << it->first << ", ";
            }
            std::cout << std::endl;
        } else {
            for(typename std::map<Key, TreePage*>::iterator it = this->pages->begin(); it != this->pages->end(); it++){
                std::cout << it->first << ", ";
            }
            std::cout << std::endl;
        }
    }

    Value* getValue(Key key){
        assert(this->isExternal());
        return &(*this->items)[key];
    }

    Key firstKey(){
        if (this->bottom) {
            return this->items->begin()->first;
        } else {
            return this->pages->begin()->first;
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
            for (int i = 0; i < half; i++) {
                page->add(it->first, it->second);
                it = this->items->erase(it);
            }
        } else {
            int half = this->pages->size() / 2;
            typename std::map<Key, TreePage*>::iterator it = this->pages->begin();
            std::advance(it, half);
            for (int i = 0; i < half; i++) {
                page->add(it->first, it->second);
                it = this->pages->erase(it);
            }
            ((TreePage*)this->lastPage())->_nextPage = NULL;
            ((TreePage*)page->firstPage())->_prevPage = NULL;
        }

        this->_nextPage = page;
        page->_prevPage = this;
        return page;
    }

    TreePage* nextPage() {
        return this->_nextPage;
    }

    TreePage* prevPage() {
        return this->_prevPage;
    }

    unsigned int count(){
        if (this->bottom) {
            return this->items->size();
        } else {
            return this->pages->size();
        }
    }

    void detach(){
        this->_nextPage = NULL;
        this->_prevPage = NULL;

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
        this->_nextPage = treePage->_nextPage;
        if(this->_nextPage != NULL)
            this->_nextPage->_prevPage = this;

        treePage->detach();

        return this;
    }

    void remove(Key key){
        if (this->bottom) {
            this->items->erase(key);
        } else {
            TreePage* page = this->pages->operator[](key);
            if(page != NULL){
                if(page->_prevPage != NULL)
                    page->_prevPage->_nextPage = page->_nextPage;
                if(page->_nextPage != NULL)
                    page->_nextPage->_prevPage = page->_prevPage;

                this->pages->erase(key);
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

template<class Key, class Value> class FlatPage : public Page<Key, Value>{
private:
    Key* keys;
    Value* values;
    FlatPage** pages;
    FlatPage* _nextPage;
    FlatPage* _prevPage;

    unsigned int size;
    unsigned int order;
    bool bottom;
    bool loaded;
    friend class Btree<Key, Value>;
public:
    FlatPage(std::string& filename){
        this->open(filename);
    }

    FlatPage(int order, bool bottom){
        this->order = order;
        this->bottom = bottom;
        this->size = 0;
        this->keys = new Key[order];
        if(!bottom){
            this->pages = new FlatPage*[order];
        } else {
            this->values = new Value[order];
        }
        this->loaded = true;
    }

    /*void save(std::string& filename){
        std::ofstream file;
        file.open(filename + ".idx");
        file.write(this->bottom ? "1" : "0", 1);
        file.write(this->size, sizeof(int));
        file.write(this->order, sizeof(int));
        //copy using memcopy
        file.write((char*)this->keys, this->size * sizeof(Key));
        if(this->bottom){
            file.write((char*)this->values, this->size * sizeof(Value));
        } else {
            for(int i=0; i<this->size; i++){
                this->pages[i]->save(filename + std::to_string(i));
            }
        }
    }

    void open(std::string& filename){
        std::ifstream file;
        file.open(filename);
        char bottom;
        file.read(&bottom, 1);
        this->bottom = bottom == '1';
        file.read(this->size, sizeof(int));
        file.read(this->order, sizeof(int));
        //copy using memcopy
        file.read((char*)this->keys, this->size * sizeof(Key));
        if(this->bottom){
            file.read((char*)this->values, this->size * sizeof(Value));
        } else {
            for(int i=0; i<this->size; i++){
                this->pages[i] = new FlatPage(filename + std::to_string(i));
            }
        }
        this->loaded = true;
    }*/

    void printKeys(){
        for(int i=0; i<this->size; i++){
            std::cout << this->keys[i] << ", ";
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
                this->pages[mid] = (FlatPage<Key, Value>*)page;
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
        this->pages[first] = (FlatPage<Key, Value>*)page;
        this->size++;
    }

    FlatPage* next(Key key){
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
        if( this->bottom) {
            int half = this->size / 2;
            memcpy(page->keys, this->keys + half, half * sizeof(Key));
            memcpy(page->values, this->values + half, half * sizeof(Value));
            this->size = half;
            page->size = half;
        } else {
            int half = this->size / 2;
            memcpy(page->keys, this->keys + half, half * sizeof(Key));
            memcpy(page->pages, this->pages + half, half * sizeof(FlatPage*));
            this->size = half;
            page->size = half;
        }
        this->_nextPage = page;
        page->_prevPage = this;
        return page;
    }

    FlatPage* nextPage() {
        return this->_nextPage;
    }

    FlatPage* prevPage() {
        return this->_prevPage;
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
        this->_nextPage = flatPage->_nextPage;
        if(flatPage->_nextPage != NULL)
            flatPage->_nextPage->_prevPage = this;

        flatPage->detach();
        delete flatPage;
        
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


    Key firstKey(){
        return this->keys[0];
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

    //Write page to disk
    void close(){
        //Write the page to disk
        std::ofstream file;
        file.open("page.txt");
        //copy using memcopy
        file.write((char*)this->keys, this->size * sizeof(Key));
        if(this->bottom){
            file.write((char*)this->values, this->size * sizeof(Value));
        } else {
            file.write((char*)this->pages, this->size * sizeof(FlatPage*));
        }
    }

    void open(){
        //Read the page from disk
        std::ifstream file;
        file.open("page.txt");
        //copy using memcopy
        file.read((char*)this->keys, this->size * sizeof(Key));
        if(this->bottom){
            file.read((char*)this->values, this->size * sizeof(Value));
        } else {
            file.read((char*)this->pages, this->size * sizeof(FlatPage*));
        }
    }

    void detach(){
        this->_nextPage = NULL;
        this->_prevPage = NULL;
        if (this->bottom) {
            this->values = NULL;
        } else {
            this->pages = NULL;
        }
        this->loaded = false;
    }

    ~FlatPage(){
        delete[] this->keys;
        if(this->bottom){
            delete[] this->values;
        } else {
            for(int i=0; i<this->size; i++){
                delete this->pages[i];
            }
            delete[] this->pages;
        }
    }

};

template<class Key, class Value> class Btree{
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
        this->root = newPage(true);
        this->root->add(sentinel, sentinelValue);
        this->height = 1;
    }

    Page<Key, Value>* newPage(bool bottom){
        if(!this->memoryOnly){
            return new FlatPage<Key, Value>(this->order, bottom);
        } else {
            return new TreePage<Key, Value>(bottom, this->order);
        }
    }

    Value* get(Key key){
        return this->get(this->root, key);
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
            this->root = newPage(false);
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
        this->deleteKey(next, key);
        if (next->count() < this->order/2){
            Page<Key, Value>* prev = next->prevPage();
            if( prev != NULL) {
                page->remove(next->firstKey());
                prev->merge(next);
                delete next;

                if (prev->isFull()) {
                    Page<Key, Value>* split_page = prev->split();
                    page->add(split_page->firstKey(), split_page);
                }
            } else {
                Page<Key, Value>* next_next = next->nextPage();
                if (next_next != NULL) {
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

    void save(){
        this->root->save("btree");
    }

    unsigned int count(){
        return this->n;
    }

    unsigned int get_height(){
        return this->height;
    }


    ~Btree(){
        //Delete the root page
        delete this->root;
    }
};
