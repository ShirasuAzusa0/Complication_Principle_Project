#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool IsDigit(char c){ return c>='0'&&c<='9'; }
bool IsAlpha(char c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }

#define _ID400 400
#define _num500 500
#define _annotation600 600
#define KEYWORD_else 100
#define KEYWORD_if 101
#define KEYWORD_int 102
#define KEYWORD_float 103
#define KEYWORD_double 104
#define KEYWORD_return 105
#define KEYWORD_void 106
#define KEYWORD_do 107
#define KEYWORD_while 108
#define OPERATOR_PLUS 200
#define OPERATOR_MINUS 201
#define OPERATOR_MULTIPLY 202
#define OPERATOR_DIVIDE 203
#define OPERATOR_MOD 204
#define OPERATOR_LT 205
#define OPERATOR_LTEQ 206
#define OPERATOR_RTEQ 207
#define OPERATOR_RT 208
#define OPERATOR_EQ 209
#define OPERATOR_NE 210
#define OPERATOR_ASSIGN 211
#define OPERATOR_SEMI 212
#define OPERATOR_DOU 213
#define OPERATOR_LLM 214
#define OPERATOR_RLM 215
#define OPERATOR_LMM 216
#define OPERATOR_RMM 217
#define OPERATOR_LBM 218
#define OPERATOR_RBM 219

static struct { const char* keyword; int encoding; } keywordTable[] = {
	{"else", 100},
	{"if", 101},
	{"int", 102},
	{"float", 103},
	{"double", 104},
	{"return", 105},
	{"void", 106},
	{"do", 107},
	{"while", 108},
};

static struct { const char* op; int encoding; } operatorTable[] = {
	{"+", 200},
	{"-", 201},
	{"*", 202},
	{"/", 203},
	{"%", 204},
	{"<", 205},
	{"<=", 206},
	{">=", 207},
	{">", 208},
	{"==", 209},
	{"!=", 210},
	{"=", 211},
	{";", 212},
	{",", 213},
	{"(", 214},
	{")", 215},
	{"[", 216},
	{"]", 217},
	{"{", 218},
	{"}", 219},
};

void concat(char str[], char tmp){ size_t len=strlen(str); str[len]=tmp; str[len+1]='\0'; }
int findKeywordEncoding(const char* str){ 
    printf("²éÕÒ¹Ø¼ü×Ö: %s\n", str);
    for(int i=0;i<sizeof(keywordTable)/sizeof(keywordTable[0]);i++) { 
        printf("  ±È½Ï: %s vs %s\n", str, keywordTable[i].keyword);
        if(strcmp(str,keywordTable[i].keyword)==0) { 
            printf("  ÕÒµ½¹Ø¼ü×Ö: %s, ±àÂë: %d\n", str, keywordTable[i].encoding);
            return keywordTable[i].encoding; 
        }
    } 
    printf("  Î´ÕÒµ½¹Ø¼ü×Ö: %s\n", str);
    return -1; 
}
int findOperatorEncoding(const char* str){ 
    printf("²éÕÒ²Ù×÷·û: %s\n", str);
    for(int i=0;i<sizeof(operatorTable)/sizeof(operatorTable[0]);i++) { 
        printf("  ±È½Ï: %s vs %s\n", str, operatorTable[i].op);
        if(strcmp(str,operatorTable[i].op)==0) { 
            printf("  ÕÒµ½²Ù×÷·û: %s, ±àÂë: %d\n", str, operatorTable[i].encoding);
            return operatorTable[i].encoding; 
        }
    } 
    printf("  Î´ÕÒµ½²Ù×÷·û: %s\n", str);
    return -1; 
}
void outputToken(FILE* fp,int encoding,const char* val){ 
    if(val&&strlen(val)>0){ 
        fprintf(fp,"%d \"%s\" ",encoding,val); 
        printf("%d \"%s\" ",encoding,val); 
    } else { 
        fprintf(fp,"%d ",encoding); 
        printf("%d ",encoding); 
    } 
}
int isFloatNumber(const char* str){ for(int i=0;str[i];i++) if(str[i]=='.') return 1; return 0; }

void coding(FILE* input_fp, FILE* output_fp){
    char tmp=fgetc(input_fp);
if(tmp == EOF) return;

    /* -------- ĞĞ×¢ÊÍ´¦Àí²¹¶¡ -------- */
    if(tmp == '/'){
        char tmp2 = fgetc(input_fp);
        if(tmp2 == '/'){
            char c;
            while((c = fgetc(input_fp)) != EOF && c != '\n');
            return;
        }else{
            ungetc(tmp2, input_fp);
        }
    }

    // Ìø¹ı¿Õ°××Ö·û
    if(tmp==' '||tmp=='\n'||tmp=='\t'||tmp=='\r') { return; }
    ungetc(tmp,input_fp);
    
    int state=0;
    bool flag=false;
    bool isIdentifier=false;
    bool isDigit=false;
    bool isFloat=false;
    char value[1024]; value[0]='\0';
    
    while(!flag){
        tmp=fgetc(input_fp);
        if(tmp == EOF) { flag=true; break; }
        
        // ¼ì²é×Ö·ûÀàĞÍ
        if(tmp >= '0' && tmp <= '9') isDigit = true;
        else if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') isIdentifier = true;
        else if(tmp == '.') isFloat = true;
        
        printf("×´Ì¬ %d, ¶ÁÈ¡×Ö·û '%c' (ASCII %d)\n", state, tmp, tmp);
        
        switch(state){
		case 0:{
			if(tmp >= '0' && tmp <= '9') {
				state=1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 0, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=5;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 0, state);
				break;
			}
			switch(tmp){
			case '+': state=4; break;
			case '(': state=4; break;
			case ')': state=4; break;
			case '*': state=4; break;
			case '[': state=4; break;
			case ']': state=4; break;
			case '!': state=2; break;
			case '%': state=4; break;
			case ',': state=4; break;
			case '-': state=4; break;
			case '.': state=-1; break;
			case '/': state=9; break;
			case ';': state=4; break;
			case '<': state=10; break;
			case '=': state=10; break;
			case '>': state=10; break;
			case '_': state=5; break;
			case 'b': state=-1; break;
			case 'd': state=8; break;
			case 'e': state=18; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=15; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=14; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=20; break;
			case 'w': state=16; break;
			case '{': state=4; break;
			case '}': state=4; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 1:{
			if(tmp >= '0' && tmp <= '9') {
				state=1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 1, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 1, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=3; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 2:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 2, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 2, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=4; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 3:{
			if(tmp >= '0' && tmp <= '9') {
				state=6;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 3, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 3, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 4:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 4, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 4, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 5:{
			if(tmp >= '0' && tmp <= '9') {
				state=5;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 5, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=5;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 5, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=5; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 6:{
			if(tmp >= '0' && tmp <= '9') {
				state=6;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 6, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 6, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 7:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 7, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 7, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 8:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 8, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 8, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=11; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 9:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 9, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 9, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=24; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 10:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 10, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 10, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=4; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 11:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 11, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 11, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=12; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 12:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 12, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 12, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=23; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 13:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 13, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 13, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=4; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 14:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 14, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 14, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=29; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 15:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 15, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 15, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=4; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=27; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 16:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 16, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 16, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=22; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 17:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 17, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 17, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=13; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 18:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 18, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 18, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=26; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 19:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 19, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 19, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=4; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 20:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 20, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 20, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=17; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 21:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 21, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 21, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=4; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 22:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 22, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 22, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=23; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 23:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 23, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 23, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=21; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 24:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 24, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 24, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=7; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 25:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 25, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 25, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=19; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 26:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 26, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 26, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=21; break;
			case 't': state=-1; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 27:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 27, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 27, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=4; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 28:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 28, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 28, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=-1; break;
			case 'u': state=25; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 29:{
			if(tmp >= '0' && tmp <= '9') {
				state=-1;
				printf("Êı×Ö×ªÒÆ: %d -> %d\n", 29, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("×ÖÄ¸×ªÒÆ: %d -> %d\n", 29, state);
				break;
			}
			switch(tmp){
			case '+': state=-1; break;
			case '(': state=-1; break;
			case ')': state=-1; break;
			case '*': state=-1; break;
			case '[': state=-1; break;
			case ']': state=-1; break;
			case '!': state=-1; break;
			case '%': state=-1; break;
			case ',': state=-1; break;
			case '-': state=-1; break;
			case '.': state=-1; break;
			case '/': state=-1; break;
			case ';': state=-1; break;
			case '<': state=-1; break;
			case '=': state=-1; break;
			case '>': state=-1; break;
			case '_': state=-1; break;
			case 'b': state=-1; break;
			case 'd': state=-1; break;
			case 'e': state=-1; break;
			case 'f': state=-1; break;
			case 'h': state=-1; break;
			case 'i': state=-1; break;
			case 'l': state=-1; break;
			case 'n': state=-1; break;
			case 'o': state=-1; break;
			case 'r': state=-1; break;
			case 's': state=-1; break;
			case 't': state=28; break;
			case 'u': state=-1; break;
			case 'v': state=-1; break;
			case 'w': state=-1; break;
			case '{': state=-1; break;
			case '}': state=-1; break;
			case '~': state=-1; break;
			default: 
				printf("Ä¬ÈÏ×ªÒÆ: Î´Æ¥Åä×Ö·û '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		default:
			printf("Ä¬ÈÏ×´Ì¬: »ØÍË×Ö·û '%c'\n", tmp);
			ungetc(tmp, input_fp);
			flag = true;
			break;
		}
		if(!flag) { 
			concat(value,tmp); 
			printf("µ±Ç°value: %s\n", value);
		}
	}
	printf("´Ê·¨µ¥ÔªÊ¶±ğÍê³É: value=%s, state=%d\n", value, state);
	if(strlen(value) > 0){
		int enc = -1;
		// Ê×ÏÈ¼ì²éÊÇ·ñÊÇ¹Ø¼ü×Ö
		int kEnc = findKeywordEncoding(value);
		if(kEnc != -1) enc = kEnc;
		// Æä´Î¼ì²éÊÇ·ñÊÇ²Ù×÷·û
		int opEnc = findOperatorEncoding(value);
		if(opEnc != -1) enc = opEnc;
		// ¸ù¾İ×Ö·ûÀàĞÍÆ¥ÅäÕıÔò±í´ïÊ½
		if(enc == -1) {
			// ±êÊ¶·û´¦Àí
			if(isIdentifier) {
				enc = _ID400;
				if(enc == -1) enc = 400; // Ä¬ÈÏ±êÊ¶·û±àÂë
			}
			// Êı×Ö´¦Àí£¨¹Ø¼ü²¹¶¡ÔÚÕâÀï£©
			else if(isDigit) {
				// ÏÈ³¢ÊÔÆ¥Åä¸¡µãÊıÕıÔò±àÂë
				if(isFloat || isFloatNumber(value)) {
					if(enc == -1) enc = 500; // Ä¬ÈÏ¸¡µãÊı±àÂë
				}
				// ·ñÔò°´ÕûÊı´¦Àí
				else {
					enc = 500; // Ä¬ÈÏÕûÊı±àÂë
				}
			}
		}
		outputToken(output_fp, enc, value);
	}
}

int main(int argc,char* argv[]){
    FILE* input_fp=fopen("_sample.tny","r");
    if(input_fp==NULL){printf("Failed to open input file\n");return 1;}
    FILE* output_fp=fopen("E:/test/output.lex","w");
    if(output_fp==NULL){printf("Failed to open output file\n");fclose(input_fp);return 1;}
    
    printf("¿ªÊ¼´Ê·¨·ÖÎö...\n");
    
    char c;
    while((c=fgetc(input_fp))!=EOF){ 
        ungetc(c,input_fp); 
        printf("--- ¿ªÊ¼´¦ÀíÏÂÒ»¸ö´Ê·¨µ¥Ôª ---\n");
        coding(input_fp,output_fp); 
    }
    fprintf(output_fp,"-1"); printf("\n´Ê·¨·ÖÎöÍê³É£¡\n"); fclose(input_fp); fclose(output_fp);
    return 0;
}
