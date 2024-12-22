#include <map>
#include <fstream>
#include <cstring>
#include <cassert>

template<class Key, class Value> class Btree;

//Interface for Btree pages
template<class Key, class Value> class Page{
public:
    virtual Value* getValue(Key key) = 0;
    virtual Key firstKey() = 0;
    virtual void add(Key key, Value value) = 0;
    virtual void add(Key key, Page* page) = 0;
    virtual bool isExternal() = 0;
    virtual Page* next(Key key) = 0;
    virtual bool isFull() = 0;
    virtual Page* split() = 0;
//    virtual ~Page() = 0;
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
        }
        return page;
    }


    ~TreePage(){
        if (this->bottom) {
            delete this->items;
        } else {
            delete this->pages;
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
        return page;
    }

    Key firstKey(){
        return this->keys[0];
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

    ~FlatPage(){
        delete[] this->keys;
        delete[] this->values;
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

    void save(){
        this->root->save("btree");
    }

    ~Btree(){
        //delete this->root;
    }
};
