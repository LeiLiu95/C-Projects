#include <stdlib.h>

#include "myVector.h"

struct vvector_str
{
    int length;     // length of elements
    int size;       // # of total spots
    void** array;   // array of void ptrs
    void (*del)(void *);    // function to call deletors
};

VVector* VVector_new(int vectSize){ //func to create a new vector
    VVector* tempVector = malloc(sizeof(VVector));         // Allocate vvector
    (*tempVector).length = 0;
    (*tempVector).size = vectSize;
    (*tempVector).array = malloc(sizeof(void*) * vectSize);    // Allocate array space
    for(int i = 0; i < vectSize; i+=1){
        (*tempVector).array[i] = 0x0;    // Null init
    }
    (*tempVector).del = free;
    return tempVector;
}

VVector* VVector_new_reg(int vectSize, void (*func)(void *)){ //initializes a new vector
    VVector* tempVector = VVector_new(vectSize);    //sets the vectors size
    VVector_registerDelete(tempVector, func); 
    return tempVector;  //returns the new vector
}

void VVector_delete(VVector* deleteVector){   //function to delete/free vector
    VVector_deleteLite(deleteVector);
}

void VVector_registerDelete(VVector *tempVector, void (*func)(void *)){   //registers a new vector
    (*tempVector).del = func;
}

void VVector_deleteLite(VVector* sDeleteVector){    //function to delete the vector
    free((*sDeleteVector).array);   //free the array of the pointer
    free(sDeleteVector);          //free pointer that points to the vector
}

void VVector_deleteFull(VVector* fullVector){ //vector delete functions
    int vectorSize = (*fullVector).length;
    for(int i = 0; i < vectorSize; i+=1){   //iterate through to delete each element in vector
        if((*fullVector).array[i] != 0){
            (*fullVector).del((*fullVector).array[i]);
        }
    }
    VVector_deleteLite(fullVector);
}

void VVector_realloc(VVector* smallVector, int vectorSize){
    if(vectorSize <= (*smallVector).length){    //if enough memory no reason to malloc
        return;
    }
    void** smallArray = (*smallVector).array;   //pointer to old array
    void** doubledArray = malloc(sizeof(void*) * vectorSize);   //make new memory
    int i = 0;
    for(; i < (*smallVector).size; i+=1){   //gets all the old arrays values
        doubledArray[i] = smallArray[i]; 
    }
    for(; i < vectorSize; i+=1){    //adds in new array values
        doubledArray[i] = 0x0;
    }

    (*smallVector).array = doubledArray;    //Set the new array
    (*smallVector).size = vectorSize;    //Set new size
    free(smallArray); //Frees the old array
}

void VVector_push(VVector* pushedVector, void* vectorPointer){  //basic push function
    if((*pushedVector).length == (*pushedVector).size){
        VVector_realloc(pushedVector,(*pushedVector).size*2);
    }
    (*pushedVector).array[(*pushedVector).length] = vectorPointer;
    (*pushedVector).length+=1;                  
}

void* VVector_pop(VVector* poppedVector){   //basic pop function for vector
    if((*poppedVector).length > 0){ //pops the vector and returns it
        (*poppedVector).length -= 1;
        return (*poppedVector).array[(*poppedVector).length]; //get the last value; lol already decremented for this
    }
    return 0x0;
}

void* VVector_get(VVector* vectorToGet, int index){    //function to grab and return a vector
    if(index < (*vectorToGet).length){
        return (*vectorToGet).array[index];
    }
    return 0x0;
}

int VVector_find(VVector* vectorToFind, void * empty){  //function to find and return the index of a vector in the array
    void **vectorArray = (*vectorToFind).array; //pointer to an array
    for(int i = 0; i < (*vectorToFind).length; i+=1){   //iterate through to find the right vector
        if(empty == vectorArray[i]){
            return i;
        }
    }
    return -1;
}

void VVector_removeAt(VVector* vectorDelete, int arrayRemoveIndex){    //function to remove a specific vector at an index
    if(arrayRemoveIndex < 0 || (*vectorDelete).length <= arrayRemoveIndex || (*vectorDelete).length < 1){   //if it does not contain vectors then nothing can be done
        return;
    }
    void **vectorArray = (*vectorDelete).array; //create an array pointer
    for(int i = arrayRemoveIndex; i < (*vectorDelete).length-1; i+=1){
        vectorArray[i] = vectorArray[i+1];
    }
    vectorArray[(*vectorDelete).length-1] = NULL;
}

void VVector_remove(VVector* vectorToRemove, void * item){   //function to remove vector
    VVector_removeAt(vectorToRemove, VVector_find(vectorToRemove, item));
}

const void * const * VVector_toArray(VVector* vectorArray){    //function to return array of the vector
    return (*vectorArray).array;
}

void** VVector_toArray_cpy(VVector* toArrayVectorCopy){
    void** copyArray = malloc(sizeof(void*) * (*toArrayVectorCopy).length);
    void** array = (*toArrayVectorCopy).array;
    for(int i = 0; i < (*toArrayVectorCopy).length; i+=1){
        copyArray[i] = array[i];
    }
    return copyArray;
}

int VVector_size(VVector* passedVector){    //returns the size of the vector
    return (*passedVector).size;
}

int VVector_length(VVector* passedVector){  //returns the length of the vector
    return (*passedVector).length;
}
