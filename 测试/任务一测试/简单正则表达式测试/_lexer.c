#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool IsDigit(char c){ return c>='0'&&c<='9'; }
bool IsAlpha(char c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }

#define _FLOAT100 100
#define _ID200 200

static struct { const char* keyword; int encoding; } keywordTable[] = {
};

static struct { const char* op; int encoding; } operatorTable[] = {
};

void concat(char str[], char tmp){ size_t len=strlen(str); str[len]=tmp; str[len+1]='\0'; }
int findKeywordEncoding(const char* str){ 
    printf("查找关键字: %s\n", str);
    for(int i=0;i<sizeof(keywordTable)/sizeof(keywordTable[0]);i++) { 
        printf("  比较: %s vs %s\n", str, keywordTable[i].keyword);
        if(strcmp(str,keywordTable[i].keyword)==0) { 
            printf("  找到关键字: %s, 编码: %d\n", str, keywordTable[i].encoding);
            return keywordTable[i].encoding; 
        }
    } 
    printf("  未找到关键字: %s\n", str);
    return -1; 
}
int findOperatorEncoding(const char* str){ 
    printf("查找操作符: %s\n", str);
    for(int i=0;i<sizeof(operatorTable)/sizeof(operatorTable[0]);i++) { 
        printf("  比较: %s vs %s\n", str, operatorTable[i].op);
        if(strcmp(str,operatorTable[i].op)==0) { 
            printf("  找到操作符: %s, 编码: %d\n", str, operatorTable[i].encoding);
            return operatorTable[i].encoding; 
        }
    } 
    printf("  未找到操作符: %s\n", str);
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
    // 跳过空白字符
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
        
        // 检查字符类型
        if(tmp >= '0' && tmp <= '9') isDigit = true;
        else if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') isIdentifier = true;
        else if(tmp == '.') isFloat = true;
        
        printf("状态 %d, 读取字符 '%c' (ASCII %d)\n", state, tmp, tmp);
        
        switch(state){
		case 0:{
			if(tmp >= '0' && tmp <= '9') {
				state=3;
				printf("数字转移: %d -> %d\n", 0, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=1;
				printf("字母转移: %d -> %d\n", 0, state);
				break;
			}
			switch(tmp){
			case '.': state=-1; break;
			default: 
				printf("默认转移: 未匹配字符 '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 1:{
			if(tmp >= '0' && tmp <= '9') {
				state=1;
				printf("数字转移: %d -> %d\n", 1, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=1;
				printf("字母转移: %d -> %d\n", 1, state);
				break;
			}
			switch(tmp){
			case '.': state=-1; break;
			default: 
				printf("默认转移: 未匹配字符 '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 2:{
			if(tmp >= '0' && tmp <= '9') {
				state=4;
				printf("数字转移: %d -> %d\n", 2, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("字母转移: %d -> %d\n", 2, state);
				break;
			}
			switch(tmp){
			case '.': state=-1; break;
			default: 
				printf("默认转移: 未匹配字符 '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 3:{
			if(tmp >= '0' && tmp <= '9') {
				state=3;
				printf("数字转移: %d -> %d\n", 3, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("字母转移: %d -> %d\n", 3, state);
				break;
			}
			switch(tmp){
			case '.': state=2; break;
			default: 
				printf("默认转移: 未匹配字符 '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		case 4:{
			if(tmp >= '0' && tmp <= '9') {
				state=4;
				printf("数字转移: %d -> %d\n", 4, state);
				break;
			}
			if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {
				state=-1;
				printf("字母转移: %d -> %d\n", 4, state);
				break;
			}
			switch(tmp){
			case '.': state=-1; break;
			default: 
				printf("默认转移: 未匹配字符 '%c'\n", tmp);
				flag=true; ungetc(tmp,input_fp); break;
			}
			break;
		}
		default:
			printf("默认状态: 回退字符 '%c'\n", tmp);
			ungetc(tmp, input_fp);
			flag = true;
			break;
		}
		if(!flag) { 
			concat(value,tmp); 
			printf("当前value: %s\n", value);
		}
	}
	printf("词法单元识别完成: value=%s, state=%d\n", value, state);
	if(strlen(value) > 0){
		int enc = -1;
		// 首先检查是否是关键字
		int kEnc = findKeywordEncoding(value);
		if(kEnc != -1) enc = kEnc;
		// 其次检查是否是操作符
		int opEnc = findOperatorEncoding(value);
		if(opEnc != -1) enc = opEnc;
		// 根据字符类型匹配正则表达式
		if(enc == -1) {
			// 标识符处理
			if(isIdentifier) {
				enc = _ID200;
				if(enc == -1) enc = 400; // 默认标识符编码
			}
			// 数字处理（关键补丁在这里）
			else if(isDigit) {
				// 先尝试匹配浮点数正则编码
				if(isFloat || isFloatNumber(value)) {
					enc = _FLOAT100;
					if(enc == -1) enc = 500; // 默认浮点数编码
				}
				// 否则按整数处理
				else {
					enc = 500; // 默认整数编码
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
    
    printf("开始词法分析...\n");
    
    char c;
    while((c=fgetc(input_fp))!=EOF){ 
        ungetc(c,input_fp); 
        printf("--- 开始处理下一个词法单元 ---\n");
        coding(input_fp,output_fp); 
    }
    fprintf(output_fp,"-1"); printf("\n词法分析完成！\n"); fclose(input_fp); fclose(output_fp);
    return 0;
}
