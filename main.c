#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static FILE *source_fp, *dest_fp;

// <Token-Type,Token-Value>
struct pair {
    char* label;
    char value[20]; // Maximum number of accepted word length is 20
};

struct pair pairs[10][30];
int k = 0, l = 0;           // For the array of structs (pairs)
int bufferCnt = 0;          // Buffer Counter for concatenating chars and cutting them


/* ....................... Checking if a given word is a KeyWord or not .....................*/ 
int isKeyword(char buffer[]){

    char keywords[38][10] = {"int", "long", "float", "string", "False","double", "class", "finally", "is", "return", "None", "continue",
                             "for", "lambda", "try", "and", "del", "global", "not", "with",
                             "True", "def", "from", "nonlocal", "while", "as", "elif",
                             "if", "or", "yield", "assert", "else", "import",
                             "pass", "break", "except", "in", "raise"};

    int i, flag = 0; // the return value which would let us know if it's a reserved keyword or not

    // Checking if the word is a keyword or not using STRCMP BuiltIn Function

    for (i = 0; i < 32; ++i)
    {
        if (strcmp(keywords[i], buffer) == 0)
        {
            flag = 1;
            break;
        }
    }

    return flag;
}


/* ....................... Removing End of Line .....................*/ 
void removeEndofLine(char line[]){
    int i = 0;
    while (1)
    {
        if (line[i] != '\n')
            i++;
        else
            break;
    }
    line[i] = '\0';
}

/* ....................... Checking the file Extension (.Hapd) .....................*/ 
void extention(const char path[]){
    int i = 0;
    char check[5];
    while (1)
    {
        if (path[i] == '.')
        {
            check[0] = path[i + 1];
            check[1] = path[i + 2];
            check[2] = path[i + 3];
            check[3] = path[i + 4];
            check[4] = '\0';
            if (strcmp(check, "hapd") != 0)
            {
                printf("Error in your file extension, your file extension is %s ,please make sure to make it .hapd\n", check);
                exit(0);
            }
            else
                break;
        }
        i++;
    }
}

/* ....................... Checking - Single Operators - .....................*/ 
int isOp(char *c){
    char operators[] = "+-*/%=><!";
    int i = 0;
    for (i = 0; i < 9; i++)
    { // 6 to be replaced with size of operators array
        if (*c == operators[i])
            return 1;
    }
    return -1;
}

/* ....................... Handling the file Path and format .....................*/ 
char* returnPath(){
    FILE *fp;
    // Enter PATH of file which want to compile
    char PATH[100];
    printf("Enter Full PAth to your file\n");
    fgets(PATH, 100, stdin);
    removeEndofLine(PATH); // remove \n from input file
    extention(PATH);
    return PATH;
}

/* ....................... Handling Inputs (.Hapd/Path/StdIn) .....................*/ 
void initIo(FILE **fp, FILE *std, const char mode[], const char fn[]) {
    if (fn[0] == '\0')
        *fp = std;
    else if(fn[0] == '-' && fn[1] == 'p'){
        // From a certain given path
        fn = returnPath();                  // File Pointer to open the file needed..
        if ((*fp = fopen(fn, mode)) == NULL)
            printf("Failed to open file");
    }
    else{
        // From argv (in the same folder)
        extention(fn);
        if ((*fp = fopen(fn, mode)) == NULL)
            printf("Failed to open file");
    }
}

/* ....................... Checking for a KeyWord or Identfier .....................*/ 
void isKeyIdnt(char buffer[]){
    char doubleOperators[6][5] = {"gr", "sm", "greq", "smeq", "eq", "neq"};
    if (buffer[0] == '\0'){}
    else if (isKeyword(buffer) == 1){
    	pairs[k][l] = (struct pair){"keyword","\0"};
        strcpy(pairs[k][l].value, buffer);
        l++;
    }
    else if (isKeyword(buffer) == 0){
        int i, op_flag = 0;
        for(i =0; i < 6; i++)
        {
            if(!strcmp(buffer, doubleOperators[i])){
            pairs[k][l] = (struct pair){"operator","\0"};
            strcpy(pairs[k][l].value, buffer);
            l++;
            op_flag = 1;
            break;
            }
        }
        if(op_flag == 0)
        {
    	pairs[k][l] = (struct pair){"identifier","\0"};
        strcpy(pairs[k][l].value, buffer);
        l++;
        }
    }
}

/* ....................... Checking for - Double Operators - .....................*/ 
void checkOperators (char *ch, char buffer[]){

    int first = 0; // To catch the operators
    first = isOp(ch);
    if (first == 1)   //first char is operator
    {
    	isKeyIdnt(buffer);
        bufferCnt = 0;
        char ch2 = fgetc(source_fp);
        int second = isOp(&ch2);

        if (second == 1)
        {
            char ch3 = fgetc(source_fp);
            int third = isOp(&ch3);
            if(third == 1) printf("There is no such operator as: %c%c%c\n", *ch, ch2, ch3);
            else
            {
                if ((ch2 == '=' && (*ch == '+' || *ch == '-' || *ch == '<' || *ch == '>')) || (ch2 == '=' && *ch == '=') || (ch2 == '+' && *ch == '+') || (ch2 == '-' && *ch == '-'))
                {
                    char op[2];
                    op[0] = *ch;
                    op[1] = ch2;
                    pairs[k][l] = (struct pair){"double operator",""};  /// check with strcat()
                    strcpy(pairs[k][l].value, op);
                    l++;
                }
                *ch = ch3;
            }
        }
        else
        {
        	pairs[k][l] = (struct pair){"single operator",*ch};
        	l++;
            *ch = ch2;
        }
    }
}

/* ....................... Checking for - Special Characters - .....................*/ 
void checkSpecial(char *ch, char buffer[]){
    int i;
    char special[13]="[]{},.();:'\"\?";
    for (i = 0; i < 13; ++i)
    {
        if (*ch == special[i])
        {
            if (bufferCnt != 0)
            {
            	bufferCnt = 0;
                isKeyIdnt(buffer);
            }
            pairs[k][l] = (struct pair){"special character",*ch};
            l++;
        }
    }
}

/* ....................... Checking for - Special Characters - .....................*/ 
void checkAll(){
    char ch;
    char buffer[20];
    int i;
    while ((ch = fgetc(source_fp)) != EOF)
    {
        checkOperators(&ch, buffer);
        checkSpecial(&ch, buffer);
        if (isalnum(ch))
    	{
        	buffer[bufferCnt++] = ch;
        	buffer[bufferCnt] = '\0';  // Added this to modify the buffer dynamically
    	}
        if ((ch == ' ' || ch == '\n' ) && (bufferCnt != 0))
        {
            isKeyIdnt(buffer);
            bufferCnt = 0;
            buffer[bufferCnt] = '\0';  // To check with it in the isKeyIdnt for nulls
        }

        if(ch=='\n'){
        	k++;
        	l++;
	    	pairs[k][l] = (struct pair){'\0', '\0'};
	    	l=0;
	    	// I will add this to exit when pressing enter in stdin mode to test the output of (pairs)
	    	//break;
        }


    }
    if(bufferCnt != 0)isKeyIdnt(buffer);
    int j=0;
}

char a[40][20];
int  top = -1;


void syntaxError(){
printf("\nSyntax Error\n\n");
}


void push(char k[])
{
    int i = 0;
    char* args[10];
    args[i] = strtok(k," ");    // K represents the rule
    while(args[i] != NULL){
        i++;
        args[i] = strtok(NULL," ");
    }

    for(i = i-1; i >= 0; i--){
        top++;
        strcpy(a[top], args[i]);
    }
}
char* TOS(){        //Returns TOP of the Stack
  return a[top];
}

void pop(){       //Pops 1 element from the Stack
  strcpy(a[top],"\0");
  top--;
}

  char non_terminal[31][5]={"S","D","AT","ST","CT","BT","DSC","AS","X","ET","ARE","A","AP","B","DP","C","SE","BE","ALE","DSE","DT","EX","IFE","CON","V","OP","LE","WE","FE","I","IC"};
  char terminal[42][20]={";","int","float","double","long","string","char","bool","stack","queue","vector","identifier","=","[number]","[number][number]","+","-","*","/","number","(",")","letters","true","false","character","<",">","if",":","else:","finish","gr","sm","greq","smeq","eq","neq","while","for","in","$"};
  char t[31][42][30]={"$","D ;","D ;","D ;","D ;","D ;","D ;","D ;","D ;","D ;","D ;","AS ;","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","EX ;","$","e","e","$","$","$","$","$","$","EX ;","EX ;","$","e",
  "$","AT AS","AT AS","AT AS","AT AS","ST AS","CT AS","BT AS","DSC DSE","DSC DSE","DSC DSE","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","int","float","double","long","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","string","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","char","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","bool","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","stack","queue","vector","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","identifier X","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "e","$","$","$","$","$","$","$","$","$","$","$","= ET","[number]","[number][number]","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","ARE","$","$","$","$","ARE","$","$","ARE","ARE","$","SE","BE","BE","ALE","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","A","$","$","$","$","A","$","$","A","A","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","B AP","$","$","$","$","B AP","$","$","B AP","B AP","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "e","$","$","$","$","$","$","$","$","$","$","$","$","$","$","+ B AP","- B AP","$","$","$","$","e","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","C DP","$","$","$","$","C DP","$","$","C DP","C DP","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "e","$","$","$","$","$","$","$","$","$","$","$","$","$","$","e","e","* C DP","/ C DP","$","$","e","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","identifier","$","$","$","$","- C","$","$","number","( A )","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","letters","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","true","false","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","character","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","< DT > identifier","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","AT","AT","AT","AT","ST","CT","BT","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","IFE","$","$","$","$","$","$","$","$","$","LE","LE","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","if CON : S else: S finish","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","( V OP V )","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","identifier","$","$","$","$","$","$","$","number","$","$","$","$","$","character","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","gr","sm","greq","smeq","eq","neq","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","WE","FE","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","while CON : S finish","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","for I in IC : S finish","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","identifier","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$",
  "$","$","$","$","$","$","$","$","$","$","$","identifier","$","$","$","$","$","$","$","number","( IC )","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$","$"
  };

int main(int argc, char *argv[]){

    printf("\nIf you want to use the stdIn - Compiler the file then write your code \n");
    printf("then press Ctrl + D , if you want to use a textFile Hint: Must be ./a.out TextName.Hapd and in the same Directory \n");
    printf("if the file .Hapd in other directory just compile then ./a.out -p and the full path. \n \n");
    
    initIo(&source_fp, stdin,  "r",  argc > 1 ? argv[1] : "");
    int i = 0, j = 0, ir = 0, ic = 0, choice = 0;
    char* term,*nonterm,r[30];
    checkAll();
    while(1){

    
    printf("Choose what you want: Press (1 or 2) \n");
    printf("1- Lexical Analysis \n");
    printf("2- Parsing LL(1) \n \n");
    scanf("%d", &choice);
    // Lexical Analysis Phase
    if(choice == 1)
    {
        for (i = 0; i < k; i++){
            while(pairs[i][j].label != '\0'){
    		printf("%s: %s\n",  pairs[i][j].label, pairs[i][j].value);
            j++;
            }
            j = 0;
        }
        printf("\n");
    }
    // Parsing Phase
    else if (choice == 2){
    char line[10]="S $";
    push(line);
    for (i = 0; i < k; i++){
        printf("%d", k);
        while(pairs[i][j].label != '\0'){
            int n=0;
            if(!strcmp(TOS(),term)||!strcmp(TOS(),"e"))
                {
                    pop();
                    j++;
                    term=pairs[i][j].value;
                    if(!strcmp(term,"else"))
                    strcpy(term,"else:");
                }

            else if(strcmp(TOS(),"$")){
                term=pairs[i][j].value;
                if(!strcmp(term,"else"))
                        strcpy(term,"else:");
                nonterm=TOS();
                for (n=0;n<31;n++){
                    if(!strcmp(non_terminal[n],nonterm)){
                        ir=n;
                        break;
                    }
                }
                for (n=0;n<42;n++){
                    if(!strcmp(terminal[n],term)){
                        ic=n;
                        break;
                    }
                }
                if(ic!=n){
                if(((!strcmp(term,"'")|| term[0]=='"'))&&((!strcmp(pairs[i][j+2].value,"'"))||(!strcmp(pairs[i][j+2].value,"\"")))){
                    j++;
                    term=pairs[i][j].value;
                    if(strlen(term)==1){
                        term="character";
                        pairs[i][j].label="character";
                        ic=25;
                    }
                    else
                    {
                        term="letters";
                        pairs[i][j].label="letters";
                        ic=22;
                    }
                }
                else if((!strcmp(term,"'")|| !strcmp(term,"\""))){
                    j++;
                    term=pairs[i][j].value;
                    for (n=0;n<42;n++){
                    k=strcmp(terminal[n],term);
                    if(k==0){
                        ic=n;
                        break;
                    }
                }
                }
                else if(!strcmp(pairs[i][j].label,"character"))
                {
                    term="character";
                    ic=25;
                }

                else if(!strcmp(pairs[i][j].label,"letters"))
                {
                    term="letters";
                    ic=22;
                }
                else if(term[0]>='0' && term[0]<='9'){
                    int f=1;
                    int len=strlen(term);

                    for(n=0;n<len;n++){
                        if(term[n]>='0' && term[n]<='9')
                            continue;
                        else{
                            f=0;
                            break;
                        }

                    }
                    if(f==1){
                    term="number";
                    pairs[i][j].label="number";
                    ic=19;
                    }
                }

                else if(!strcmp(pairs[i][j].label,"number"))
                {
                    term="number";
                    ic=19;
                }
                else if(term[0]=='['){
                    j++;
                    term=pairs[i][j].value;
                    int len=strlen(term);
                    int f=1;
                    for(n=0;n<len;n++){
                        if(term[n]>='0' && term[n]<='9')
                            continue;
                        else{
                            f=0;
                            break;
                        }
                    }
                    if(f==1){
                        j++;
                        term=pairs[i][j].value;
                        if(!strcmp(term,"]")){
                            if(!strcmp(pairs[i][j+1].value,"[")){
                                j+=2;
                                term=pairs[i][j].value;
                                len=strlen(term);
                                f=1;
                                for(n=0;n<len;n++){
                                if(term[n]>='0' && term[n]<='9')
                                    continue;
                                else{
                                    f=0;
                                    break;
                                }
                            }
                            if(f==1){
                                j++;
                                term=pairs[i][j].value;
                                if(!strcmp(term,"]")){
                                    term="[number][number]";
                                    pairs[i][j].label="[number][number]";
                                    ic=14;
                                }
                            }
                            }
                            else{
                                term="[number]";
                                pairs[i][j].label="[number]";
                                ic=13;
                            }
                        }
                    }

                }
                else if(!strcmp(pairs[i][j].label,"[number]"))
                {
                    term="[number]";
                    ic=13;
                }
                else if(!strcmp(pairs[i][j].label,"[number][number]"))
                {
                    term="[number][number]";
                    ic=14;
                }
                else if(!strcmp(pairs[i][j].label,"identifier"))
                {
                    term="identifier";
                    ic=11;
                }
                else{
                    syntaxError();
                    break;
                }
                }
                int z=0;
                for(n=0;n<31;n++){
                    if(!strcmp(nonterm,non_terminal[n])){
                        z=1;
                        break;
                    }
                }
                if(z==1){
                printf("%s ---> ", TOS());
                pop();
                strcpy(r,t[ir][ic]);
                printf("%s", r);
                printf("\n");
                push(r);
                }
                if(!strcmp(TOS(),"e"))
                {
                    pop();
                }
            }
            if(!strcmp(TOS(),"$")&&!strcmp(term,";")){
                printf("\nGiven String is accepted \n\n");
                break;

            }
            if(!strcmp(TOS(),"$")){
                syntaxError();
                break;
            }
            }
            int z;
            for(z=0;z<top;z++){
                strcpy(a[z],"\0");
            }
            top=-1;
            strcpy(line,"S $");
            push(line);
            term="";
            j=0;
    }
    }
    }
    fclose(source_fp); // close the file}
    return 0;
}
