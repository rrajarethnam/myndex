#include "FlatPage.h"

template<class Key, class Value> class CompoundObjectsFlatPage : public FlatPage<Key, Value>{

    public:
    CompoundObjectsFlatPage(int order, bool bottom):FlatPage<Key, Value>(order, bottom){}

    CompoundObjectsFlatPage(const std::string& filename, int order){
        this->order = order;
        this->open(filename);
    }
    void save(const std::string& filename){
        std::ofstream file;
        if(this->bottom){
            file.open(filename + ".values.idx");
        } else {
            file.open(filename + ".idx");
        }
        char record_separator = 30;
        file << this->size <<  std::endl;
        //copy iteratively
        for(int i=0; i<this->size; i++){
            file << this->keys[i] << record_separator;
        }
        if(this->bottom){
            for(int i=0; i<this->size; i++){
                file << this->values[i] << record_separator;
            }
        } else {
            for(int i=0; i<this->size; i++){
                this->pages[i]->save(filename + "_" + std::to_string(i));
            }
        }
    }

    void open(const std::string& filename, bool fullLoad=true){
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
        
        std::string size_str;
        getline(file, size_str);
        this->size = std::stoi(size_str);
        char record_separator = 30;

        this->keys = new Key[2*this->order];
        for(int i=0; i<this->size; i++){
            std::string key_str;
            getline(file, key_str, record_separator);
            this->keys[i] = key_str;
        }
        if(this->bottom){
            this->values = new Value[2*this->order];
            for(int i=0; i<this->size; i++){
                std::string value_str;
                getline(file, value_str, record_separator);
                this->values[i] = value_str;
            }
        } else {
            this->pages = new Page<Key, Value>*[2*this->order];
            this->loaded = new bool[2*this->order];
            for(int i=0; i<this->size; i++){
                this->pages[i] = new CompoundObjectsFlatPage<Key, Value>(filename + "_" + std::to_string(i), this->order);
                this->loaded[i] = true;
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
            std::cout.flush();
        } else {
            std::cout << "[";
            for(int i=0; i<this->size; i++){
                std::cout << this->keys[i]  << ", ";
                this->pages[i]->print();
            }
            std::cout << "]";
            std::cout.flush();
        }
        std::cout.flush();
    }

    CompoundObjectsFlatPage* split(){
        CompoundObjectsFlatPage* page = new CompoundObjectsFlatPage(this->order, this->bottom);
        int half = this->size / 2 + (this->size % 2);
        int otherHalf = this->size - half;
        //memcpy(page->keys, this->keys + half, otherHalf * sizeof(Key));
        for(int i=0; i<otherHalf; i++){
            page->keys[i] = this->keys[i + half];
        }
        this->size = half;
        page->size = otherHalf;
        if( this->bottom) {
            //memcpy(page->values, this->values + half, otherHalf * sizeof(Value));
            for(int i=0; i<otherHalf; i++){
                page->values[i] = this->values[i + half];
            }
        } else {
            memcpy(page->pages, this->pages + half, half * sizeof(CompoundObjectsFlatPage*));
        }

        return page;
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
        //memmove(this->keys + first + 1, this->keys + first, (this->size - first) * sizeof(Key));
        //memmove(this->values + first + 1, this->values + first, (this->size - first) * sizeof(Value));
        for(int i=this->size; i>first; i--){
            this->keys[i] = this->keys[i-1];
            this->values[i] = this->values[i-1];
        }

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
        //memmove(this->keys + first + 1, this->keys + first, (this->size - first) * sizeof(Key));
        for(int i=this->size; i>first; i--){
            this->keys[i] = this->keys[i-1];
        }
        memmove(this->pages + first + 1, this->pages + first, (this->size - first) * sizeof(CompoundObjectsFlatPage*));
        this->keys[first] = key;
        this->pages[first] = page;
        this->size++;
    }
    
    Page<Key, Value>* merge(Page<Key, Value>* page) {
        CompoundObjectsFlatPage* flatPage = (CompoundObjectsFlatPage*)page;
        for(int i=0; i<flatPage->size; i++){
            this->keys[this->size + i] = flatPage->keys[i];
        }
        if (this->bottom) {
            //memcpy(this->keys + this->size, flatPage->keys, flatPage->size * sizeof(Key));
            //memcpy(this->values + this->size, flatPage->values, flatPage->size * sizeof(Value));
            for(int i=0; i<flatPage->size; i++){
                this->values[this->size + i] = flatPage->values[i];
            }
            this->size += flatPage->size;
        } else {
            //memcpy(this->keys + this->size, flatPage->keys, flatPage->size * sizeof(Key));
            memcpy(this->pages + this->size, flatPage->pages, flatPage->size * sizeof(CompoundObjectsFlatPage*));
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
                    //memmove(this->keys + mid, this->keys + mid + 1, (this->size - mid - 1) * sizeof(Key));
                    //memmove(this->values + mid, this->values + mid + 1, (this->size - mid - 1) * sizeof(Value));
                    for(int i=mid; i<this->size-1; i++){
                        this->keys[i] = this->keys[i+1];
                        this->values[i] = this->values[i+1];
                    }
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
                    //memmove(this->keys + mid, this->keys + mid + 1, (this->size - mid - 1) * sizeof(Key));
                    for(int i=mid; i<this->size-1; i++){
                        this->keys[i] = this->keys[i+1];
                    }
                    memmove(this->pages + mid, this->pages + mid + 1, (this->size - mid - 1) * sizeof(CompoundObjectsFlatPage*));
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


};