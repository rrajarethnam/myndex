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

template<class Key, class Value> class TreePage : public Page<Key, Value>{
private:
    bool bottom;
    std::map<Key, TreePage*>* pages;
    std::map<Key, Value>* items;
    unsigned int order;

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

template<class Key, class Value> class FlatPage : public Page<Key, Value>{
private:
    Key* keys;
    Value* values;
    FlatPage** pages;

    unsigned int size;
    unsigned int order;
    bool bottom;
    bool *loaded;
    friend class Btree<Key, Value>;
public:
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
            this->pages = new FlatPage*[2*order];
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

    void open(const std::string& filename){
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
            this->pages = new FlatPage*[2*this->order];
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
            }
            std::cout << "}";
        } else {
            std::cout << "[";
            for(int i=0; i<this->size; i++){
                std::cout << this->keys[i]  << ", ";
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

    FlatPage* nextPageOf(Page<Key, Value>* page) {
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

    FlatPage* prevPageOf(Page<Key, Value>* page) {
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
                    FlatPage* page = this->pages[mid];

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

    Btree(std::string name){
        this->memoryOnly = false;
        std::ifstream file;
        file.open(name + ".idx.meta");
        file >> this->order;
        file >> this->height;
        file >> this->n;
        file.close();

        this->root = newPage(true);
        this->root->open(name);
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

