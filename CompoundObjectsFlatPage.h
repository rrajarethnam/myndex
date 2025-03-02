#include "FlatPage.h"



template<class Key, class Value> class CompoundObjectsFlatPage : public FlatPage<Key, Value>{
public:
    CompoundObjectsFlatPage(int order, bool bottom):FlatPage<Key, Value>(order, bottom){
    }

    CompoundObjectsFlatPage(const std::string& id, int order){
        this->id = id;
        this->order = order;
        this->is_open =false;
    }
    void save(){
        if(this->dirty == false){
            return;
        }
        this->filename = this->getId();

        std::ofstream file;
        std::ofstream metafile;
        if(this->bottom){
            file.open(this->filename + ".values.idx");
        } else {
            file.open(this->filename + ".idx");
            metafile.open(this->filename + ".meta.idx");
        }

        file << this->size <<  std::endl;
        //copy iteratively
        for(int i=0; i<this->size; i++){
            file << this->keys[i] << FlatPage<Key, Value>::RECORD_SEPARATOR;
        }
        if(this->bottom){
            for(int i=0; i<this->size; i++){
                file << this->values[i] << FlatPage<Key, Value>::RECORD_SEPARATOR;
            }
        } else {
            for(int i=0; i<this->size; i++){
                this->pages[i]->save();
                metafile << this->pages[i]->getId() << FlatPage<Key, Value>::RECORD_SEPARATOR;
            }
        }
    }

    void open(bool reload=false){
        if(this->is_open && !reload)
            return;
        this->filename = this->getId();
        this->bottom = false;
        std::ifstream file;
        std::ifstream metafile;
        file.open(this->filename + ".idx");
        if(!file.is_open()){
            file.close();
            file.open(this->filename + ".values.idx");
            this->bottom = true;
            if(!file.is_open()){
                std::cout << "Error: File not found" << std::endl;
                assert(false);
            }    
        } else {
            metafile.open(this->filename + ".meta.idx");
        }
        
        std::string size_str;
        getline(file, size_str);
        this->size = std::stoi(size_str);

        this->keys = new Key[2*this->order];
        for(int i=0; i<this->size; i++){
            std::string key_str;
            getline(file, key_str, FlatPage<Key, Value>::RECORD_SEPARATOR);
            this->keys[i] = key_str;
        }
        if(this->bottom){
            this->values = new Value[2*this->order];
            for(int i=0; i<this->size; i++){
                std::string value_str;
                getline(file, value_str, FlatPage<Key, Value>::RECORD_SEPARATOR);
                this->values[i] = value_str;
            }
        } else {
            this->pages = new Page<Key, Value>*[2*this->order];
            for(int i=0; i<this->size; i++){
                std::string page_str;
                getline(metafile, page_str, FlatPage<Key, Value>::RECORD_SEPARATOR);
                this->pages[i] = new CompoundObjectsFlatPage<Key, Value>(page_str, this->order);
            }
        }
        this->is_open = true;
    }

    void print(){
        if (this->is_open == false){
            std::cout << this->id << "::<passive>" << std::endl;
            return;
        }
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
        this->open();

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
        this->dirty = true;
        page->dirty = true;

        return page;
    }

    void add(Key key, Value value){
        assert(this->isExternal());
        this->open();

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

        this->dirty = true;

    }

    void add(Key key, Page<Key, Value>* page){
        assert(!this->isExternal());
        this->open();

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

        this->dirty = true;
    }
    
    Page<Key, Value>* merge(Page<Key, Value>* page) {
        this->open();

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

        this->dirty = true;
        
        return this;
    }

    void remove(Key key){
        this->open();

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
        this->dirty = true;
    }
};