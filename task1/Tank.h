#pragma once

// It would've been lot easier to just define everything in the header file itself
// (but ig this way of doing things makes stuff easier in the long run)

template<typename T>
class Tank{
public:
    /**
     * Initializes the array with enough space to store `initial_capacity` elements
     * NOTE: This does not initialize the structure (i.e. size of the array remains 0)
     * RAISES: Raises assertion error if memory could not be allocated
    */
    Tank(std::size_t inital_capacity = 0);

    /**
     * Resizes the array to be able to store `new_capacity` elements
     * RAISES: Raises assertion error if the `new_capacity` is less than the current size
     * RAISES: Raises assertion error if memory could not be allocated
    */
    void resize(std::size_t new_capacity);

    /**
     * Returns the size of the array
    */
    std::size_t size();

    /**
     * Appends an element to the back of the array
     * NOTE: This will automatically resize the array to twice its current capacity if there is not enough space
    */
    void push_back(T item);

    /**
     * Pops the last element from the array and returns it
    */
    T pop_back();

    /**
     * Returns the element at the index
     * RAISES: Raises assertion error the array does not contain the index
    */
    T at(long long int index);

    T operator[](long long int index);

    /**
     * Frees the allocated memory
    */
    void free_array();

    ~Tank();

private:
    T* data;
    std::size_t filled;
    std::size_t max_capacity;
};

template<typename T>
T accumulate(Tank<T> tank);

#include "Tank.cpp" // This looks mad jank but its to let the compiler find where the definitions are at :)