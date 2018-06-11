#ifndef __TOK_H__
#define __TOK_H__

//Provides a tokenizer struct
struct tokenizer_str;
typedef struct tokenizer_str Tokenizer;

void tokenConstructor(Tokenizer *token, const char *characters, const char *delimiters);    //token Constructor
void tokenDestructor(Tokenizer *tokenDest); //token destructor
Tokenizer * Tokenizer_new(const char *str, const char *delimiters);
void Tokenizer_delete(Tokenizer *this);
const char * Tokenizer_next(Tokenizer *this);
int Tokenizer_hasTokens(Tokenizer *this);
int Tokenizer_numTokens(Tokenizer *this);

#endif
