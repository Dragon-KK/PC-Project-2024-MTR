#include <cassert>
#include "Tank.h"

#define TANK_DEF(return_type) template<typename T> return_type Tank<T>::

TANK_DEF() Tank(std::size_t initial_capacity){
    max_capacity = initial_capacity;
        
    data = static_cast<T*>(malloc(max_capacity * sizeof(T)));
    assert(!(data == nullptr && max_capacity > 0));

    filled = 0;
}

TANK_DEF(void) resize(std::size_t new_capacity){
    assert(filled <= new_capacity);
        
    max_capacity = new_capacity;
    T* new_data = static_cast<T*>(realloc(data, max_capacity * sizeof(T)));
    assert(!(new_data == nullptr && max_capacity > 0));

    // NOTE That data will not be freed if new_data is not allocated (So if you were to remove the assertion do handle this)
    data = new_data;
}

TANK_DEF(std::size_t) size(){
    return filled;
}

TANK_DEF(void) push_back(T item){
    if (filled >= max_capacity){
        resize((max_capacity > 0)? (2 * max_capacity) : 1);
    }
    data[filled++] = item;
}

TANK_DEF(T) pop_back(){
    assert(filled > 0);
    return data[--filled];
}

TANK_DEF(T) at(long long int index){
    assert(index < filled);
    return data[index];
}

TANK_DEF(T) operator[](long long int index){
    return at(index);
}

TANK_DEF(void) free_array(){
    filled = 0;
    max_capacity = 0;
    free(data);
    data = nullptr;
}

TANK_DEF() ~Tank(){
    free_array();
}

template<typename T>
T accumulate(Tank<T> tank){
    T sum{};
    for(long long int i = 0; i < tank.size(); ++i){
        sum += tank[i];
    }
    return sum;
}
