#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "myVector.h"

struct tokenizer_str{
    int pos;            //Current position: sits on the next token
    int length;         //Number of elements
    char **elements;    //Array of strings
};


void tokenConstructor(Tokenizer *token, const char *characters, const char *delimiters){    //token Constructor
    if(token == NULL){  //if nothing to create then return
        return;
    }
    VVector* vectorTemp = VVector_new(1);   //else initialize and return new token
    char *charBuffer = strdup(characters);     // have a working buffer
    char* tokenPointer = strtok(charBuffer,delimiters);
    while(tokenPointer != NULL){
        VVector_push(vectorTemp,strdup(tokenPointer));  //Push the string into the vector
        tokenPointer = strtok(NULL,delimiters);          //next token
    }
    (*token).pos = 0;   //get position of the stack
    (*token).length = VVector_length(vectorTemp);                 //get the size of the array returned
    (*token).elements = (char**)VVector_toArray_cpy(vectorTemp);  //get the array of strings
    VVector_delete(vectorTemp);    //free the elements on the vector
    free(charBuffer);
}

void tokenDestructor(Tokenizer *tokenDest){ //token destructor
    if(tokenDest == NULL){  //if passed token is null then return
        return;
    }
    char **tokenStrings = (*tokenDest).elements;    //else free each token
    for(int i = 0; i < (*tokenDest).length; i+=1){  //iterate through and delete each array
        free(tokenStrings[i]);
    }
    free(tokenStrings); //free the array of arrays
}

Tokenizer* Tokenizer_new(const char *characters, const char *delimiters){
    Tokenizer *tokenTemp = malloc(sizeof(Tokenizer));

    tokenConstructor(tokenTemp, characters, delimiters);

    return tokenTemp;
}

void Tokenizer_delete(Tokenizer *tokenDelete){
    if(tokenDelete == NULL){
        return;
    }
    tokenDestructor(tokenDelete);
    free(tokenDelete);
}

const char* Tokenizer_next(Tokenizer* nextTokenRead){   //grabs next string of token an returns null if no more tokens
    char* output;
    if(nextTokenRead == NULL){  //return null if no more tokens or out of bounds
        return NULL;
    }
    else if((*nextTokenRead).pos >= (*nextTokenRead).length){
        return NULL;
    }
    else{   //incremend index and return token
        output = (*nextTokenRead).elements[(*nextTokenRead).pos];
        (*nextTokenRead).pos+=1;
    }
    return output;
}

int Tokenizer_hasTokens(Tokenizer* hasToken){   //checks if there is any remaining tokens
    if((*hasToken).pos < (*hasToken).length){   //return 1 if true or 0 for false
        return 1;
    }
    return 0;
}

int Tokenizer_numTokens(Tokenizer* numTokens){   //returns the number of tokens left
    return (*numTokens).length;
}