#include<iostream>
#include<string>
#include<fstream>          //for file handling
#include<unordered_map>   //for storing error msgs
#include<unordered_set>
#include<vector>


using namespace std;




enum ErrorType {
    INVALID_IDENTIFIER,
    INVALID_NUM,
    UNCLOSED_STRING,
    UNKNOWN_SYMBOL,
    UNTERMINATED_MULTI_LINE_COMMENT,
    UNEXPECTED_COMMENT_TERMINATOR
};


enum State {
    START,
    IN_ID,
    IN_NUM,
    IN_FLOAT,
    IN_STRING,
    IN_COMMENT_SINGLE,
    IN_COMMENT_MULTI,
    DONE,
    ERROR_STATE
};



struct token{
  int lineNo;
  string type;
  string tokenStr;

};

struct error{
    int lineNo;
ErrorType errType;

};

unordered_map<ErrorType, string> ErrorMessages = {
    { INVALID_IDENTIFIER, "Invalid identifier" },
    { INVALID_NUM, "Invalid num" },
    { UNCLOSED_STRING, "Unclosed string literal" },
    { UNKNOWN_SYMBOL, "Unknown symbol" },
    {UNTERMINATED_MULTI_LINE_COMMENT,"The multi-line comment is not terminated"},
       { UNEXPECTED_COMMENT_TERMINATOR,"Unexpected comment terminator (*#) without opening #*"}

};

unordered_set<string> reservedWords = {
    "num", "str", "bool", "if", "else", "loop", "print",
    "def", "return", "start", "end","true","false"
};

vector<token> tokenVec;
vector<error> errorVec;
vector<string> errorSymbols;

//helper functions 

void addError(int lineNo,ErrorType type){

errorVec.push_back({lineNo,type});

}

void addToken(int lineNo,string typ,string tk){

tokenVec.push_back({lineNo,typ,tk});

}


bool isOperatorChar(char c) {
    string ops = "+-*/%=<>!";
    return (ops.find(c) != string::npos);
}

bool isSeparator(char c) {
    string seps = "[](),~";
    return (seps.find(c) != string::npos);
}

 string whichOperator(const string& token) {
    static const unordered_map<string, string> operatorMap = {
        {"+", "ADD_OP"},
        {"-", "SUB_OP"},
        {"*", "MULTIPLICATIVE_OP"},
        {"/", "DIVISION_OP"},
        {"%", "MODULUS_OP"},
        {"==", "EQUAL_OP"},
        {"!=", "NOT_EQUAL_OP"},
        {"<", "LESS_THAN_OP"},
        {"<=", "LESS_EQUAL_OP"},
        {">", "GREATER_THAN_OP"},
        {">=", "GREATER_EQUAL_OP"},
        {"=", "ASSIGNMENT_OP"},
        {"!", "NOT_OP"}
    };

    auto it = operatorMap.find(token);
    if (it != operatorMap.end()) {
        return it->second;
    }
    return "UNKNOWN_OP"; // graceful fallback for unrecognized tokens
}


 
 string whichSeparator(const string& token){

  static const unordered_map<string, string> operatorMap = {
       {
       
    {"(", "LPAREN"},
    {")", "RPAREN"},
    {"[", "LBRACKET"},
    {"]", "RBRACKET"},
    {",", "COMMA"},
    {"~", "STATEMENT_TERMINATOR"}
   
}

    };

    auto it = operatorMap.find(token);
    return (it->second);

 }

 void tokenizeLine(const string &line,int  lineNo,bool& inMultiLineComm){



  State state = START;
    string tokenStr = "";
   for (size_t i = 0; i <= line.size(); ++i) {
       char c = (i < line.size()) ? line[i] : '\0';  //so at the end of line we have '\0'


//        // if we are already inside a multiline comment, keep searching for *#
// if (inMultiLineComm) {
//     size_t endPos = line.find("*#");
//     if (endPos != string::npos) {
//         // if found closing tag on this line then
//         inMultiLineComm = false;
//         tokenizeLine(line.substr(endPos + 2), lineNo, inMultiLineComm);
//     }
//     return;
// }

  // If we are currently inside a multiline comment, look for the terminator "*#"
        if (inMultiLineComm) {
            size_t endPos = line.find("*#", i);
            if (endPos != string::npos) {
                // Found terminator on this line. Move index to just after the terminator and continue processing remainder.
                inMultiLineComm = false;
                i = endPos + 1; // loop will increment i further
                continue;
            } else {
                // No terminator on this line. Entire rest of line is inside comment.
                return;
            }
        }

         switch (state) {
        case START:
            if (isspace(c)) continue;

             else if (isalpha(c)|| c == '_') {
                 tokenStr = c;
                state = IN_ID;
            }

        

  else if (isdigit(c)) {
               tokenStr = c;
                state = IN_NUM;
            }

            else if (c == '"') {
               tokenStr = c;
                state = IN_STRING;
            }

            else if (c == '#') {
                if (i + 1 < line.size() && line[i + 1] == '*') {
                    state = IN_COMMENT_MULTI;
                    i++; // skip '*'
                } else {
                    state = IN_COMMENT_SINGLE;
                }
            } 
           //for handling unexpected comment terminator symbol
   else if (c == '*') {

        if (i + 1 < line.size() && line[i + 1] == '#') {
              ErrorType err =UNEXPECTED_COMMENT_TERMINATOR;
               errorSymbols.push_back("");
                addError(lineNo, err);
                addToken(lineNo,ErrorMessages[err],"");
            i++; // skip the #
        } else {
            addToken(lineNo, "*","MULTIPLICATIVE_OP");
        }
    }
            else if (isOperatorChar(c)) {
               tokenStr = c;
                // check for double operator (==, !=, <=, >=)
                if (i + 1 < line.size() && isOperatorChar(line[i + 1])) {
                    string two =tokenStr + line[i + 1];
                    if (two == "==" || two == "!=" || two == "<=" || two == ">=") {
                        string op= whichOperator(two);
                        addToken(lineNo, two, op);
                        i++;
                        continue;
                    }
                }
                 string op= whichOperator(tokenStr);
                addToken(lineNo,tokenStr,op);
            }

            else if (isSeparator(c)) {
               tokenStr = c;
               string op=whichSeparator(tokenStr);
                addToken(lineNo,tokenStr, op);
            }

            else if (c == '\0') {
                continue;
            }

        
            else {
                ErrorType err =UNKNOWN_SYMBOL;
              //  errorSymbols.push_back(to_string(c));
               errorSymbols.push_back(string(1,c));
                addError(lineNo, err);
                 addToken(lineNo,ErrorMessages[err],string(1,c));
            }
            break;


case IN_ID:
            if (isalnum(c) || c == '_') {
               tokenStr += c;
            } else {
                if (reservedWords.count(tokenStr))  //count function either returns 1(exsists) or 0(doesn't exsist) 
                    addToken(lineNo,tokenStr, "RESERVED_WORD");
                else
                    addToken(lineNo,tokenStr, "IDENTIFIER");
               tokenStr = "";
                state = START;
                i--; // reprocess current char
            }
            break;

       case IN_NUM:
    if (isdigit(c)) {
       tokenStr += c;
    }

   
    else if (c == '.') {
       tokenStr += c;
        state = IN_FLOAT;
    }
    else if (isalpha(c)) {
        // Invalid numeric identifier like 3while or 45abc
       tokenStr += c;
        // consume rest of alphanumeric junk to get full invalidtokenStr
        size_t j = i + 1;
        while (j < line.size() && (isalnum(line[j]) || line[j] == '_'||((!isOperatorChar(line[j]))&&(!isSeparator(line[j]))))) {
           tokenStr += line[j];
            j++;
        }
        errorSymbols.push_back(tokenStr);
        ErrorType err=INVALID_IDENTIFIER;
        addError(lineNo, err);
            addToken(lineNo,ErrorMessages[err],tokenStr);
        i = j - 1;      // jump ahead so we don't reprocess junk
       tokenStr = "";
        state = START;
    }
    else {
        addToken(lineNo,tokenStr, "INTEGER");
       tokenStr = "";
        state = START;
        i--;
    }
    break;

        case IN_FLOAT:
            if (isdigit(c))tokenStr += c;

   

             else if (c == '.') {
        // Second '.' detected → gather full invalid numeric junk
        tokenStr += c;
        size_t j = i + 1;
        bool anyAlpha;
        while (j < line.size() && (isalnum(line[j]) || line[j] == '.' || line[j] == '_'||((!isOperatorChar(line[j]))&&(!isSeparator(line[j]))))) {
                if (isalpha(line[j]) ||  line[j] == '_')
                {
                    anyAlpha=true;
                }
            tokenStr += line[j];
            j++;
        }
        ErrorType err;
        if(anyAlpha){ err= INVALID_IDENTIFIER; }
        else{ err = INVALID_NUM;}

        errorSymbols.push_back(tokenStr);    
        addError(lineNo, err);
            addToken(lineNo,ErrorMessages[err],tokenStr);
        tokenStr = "";
        state = START;
        i = j - 1; // jump ahead to last junk character
    }
    
    


else if (isalpha(c)) {
        // Invalid numeric identifier like 3.9while or 45abc
       tokenStr += c;
        // consume rest of alphanumeric junk to get full invalidtokenStr
        size_t j = i + 1;
        while (j < line.size() && (isalnum(line[j]) || line[j] == '_'||((!isOperatorChar(line[j]))&&(!isSeparator(line[j]))))) {
           tokenStr += line[j];
            j++;
        }
        errorSymbols.push_back(tokenStr);
        ErrorType err=INVALID_IDENTIFIER;
        addError(lineNo, err); 
            addToken(lineNo,ErrorMessages[err],tokenStr);      
         i = j - 1;      // jump ahead so we don't reprocess junk
       tokenStr = "";
        state = START;
    }

            else {
                if (tokenStr.back() == '.')
                  { errorSymbols.push_back(tokenStr);
        ErrorType err=INVALID_NUM;
        addError(lineNo, err); 
            addToken(lineNo,ErrorMessages[err],tokenStr);
            }
                else
                    addToken(lineNo,tokenStr, "INTEGER");
               tokenStr = "";
                state = START;
                i--;
            }
            break;

        case IN_STRING:
           tokenStr += c;
            if (c == '"') {
                addToken(lineNo,tokenStr, "STRING");
               tokenStr = "";
                state = START;
            } else if (c == '\0') {
                errorSymbols.push_back(tokenStr);
        ErrorType err=UNCLOSED_STRING;
        addError(lineNo, err); 
            addToken(lineNo,ErrorMessages[err],tokenStr);
                state = START;
            }
            break;

        case IN_COMMENT_SINGLE:
            // ignore until end of line
            return;

        case IN_COMMENT_MULTI:
            // we don't support nested multiline comments

            inMultiLineComm=true;
            while (i + 1 < line.size()) {
                if (line[i] == '*' && line[i + 1] == '#') {
                    i++;
                    state = START;
                     inMultiLineComm=false;
                    break;
                }
                i++;
            }
            break;

        default:
            break;
              

        }//switch ends
    }//while ends
 }//tokenizer func ends


int main(){

    string fileName;
  fstream file;
while(true){
 std::cout<<"Enter The source file name:"<<std::endl;
   // getline(std::cin,fileName);   
   fileName="codefile.txt";
  
    file.open(fileName,std::ios::in);

   if(!file.is_open()){
    std::cerr<<"The source code file can't be opened."<<std::endl;
    file.clear();  //clear error flags
   }
   else{
    break;
   }

}


  int lineNo=1;
  string line;
 

 bool inMultiLine=false;
    while (getline(file, line)) {
        tokenizeLine(line, lineNo,inMultiLine);
        lineNo++;
    }




    if(inMultiLine){
        ErrorType err=UNTERMINATED_MULTI_LINE_COMMENT;
           addError((lineNo-1),err);
           errorSymbols.push_back("");
               addToken(lineNo,ErrorMessages[err],"");

    }


    file.close();

    cout << "\n--- TOKENS ---\n";
    for (auto &t : tokenVec) {
        cout << "Line " << t.lineNo << " | " << t.type << " | " << t.tokenStr << "\n";
    }

    cout << "\n--- ERRORS ---\n";
    if (errorVec.empty()) cout << "No lexical errors.\n";
    else {
        for (int i=0;i<errorVec.size();++i) {
            cout << "Line " << errorVec[i].lineNo << " | " << ErrorMessages[errorVec[i].errType] <<
          " "<<errorSymbols[i]<<"\n";
        }
    }

    return 0;






}
