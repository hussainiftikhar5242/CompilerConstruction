#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <map>

using namespace std;

enum TokenType {
    T_INT, T_ID, T_NUM, T_IF, T_ELSE, T_RETURN, 
    T_ASSIGN, T_PLUS, T_MINUS, T_MUL, T_DIV, 
    T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE,  T_FLOAT,  T_STRING, T_ERROR, T_CHAR,T_DOUBLE,T_BOOL,
    T_CONST,

    T_SEMICOLON, T_GT, T_EOF, 
};


struct Token {
    TokenType type;
    string value;
    int line;
};

class Lexer {

    private:
        string src;
        size_t pos;
        int line;

    public:
        Lexer(const string &src) {
            this->src = src;  
            this->pos = 0;  
            this->line = 0;  
        }

        vector<Token> tokenize() {
            vector<Token> tokens;
            while (pos < src.size()) {
                char current = src[pos];

                if(current == '\n') {
                    pos++;
                    line++;
                    continue;
                }

                
                if (isspace(current)) {
                    pos++;
                    continue;
                }
                if (isdigit(current)) {
                    tokens.push_back(Token{T_NUM, consumeNumber(),line});
                    continue;
                }
               
                if (isalpha(current)) {
                    string word = consumeWord();
                    if (word == "int") tokens.push_back(Token{T_INT, word, line});
                    else if (word == "if") tokens.push_back(Token{T_IF, word, line});
                    else if (word == "else") tokens.push_back(Token{T_ELSE, word, line});
                    else if (word == "return") tokens.push_back(Token{T_RETURN, word, line});
                    else if (word == "float") tokens.push_back(Token{T_FLOAT, word, line});
                    else if (word == "string") tokens.push_back(Token{T_STRING, word, line});
                    else if (word == "char") tokens.push_back(Token{T_CHAR, word, line});
                    else if (word == "double") tokens.push_back(Token{T_DOUBLE, word, line});
                    else if (word == "bool") tokens.push_back(Token{T_BOOL, word, line});
                    else if (word == "const") tokens.push_back(Token{T_CONST, word, line});
                    else tokens.push_back(Token{T_ID, word, line});
                    continue;
                }
                
                switch (current) {
                        case '=': tokens.push_back(Token{T_ASSIGN, "=", line}); break;
                        case '+': tokens.push_back(Token{T_PLUS, "+", line}); break;
                        case '-': tokens.push_back(Token{T_MINUS, "-", line}); break;
                        case '*': tokens.push_back(Token{T_MUL, "*", line}); break;
                        case '/': tokens.push_back(Token{T_DIV, "/", line}); break;
                        case '(': tokens.push_back(Token{T_LPAREN, "(", line}); break;
                        case ')': tokens.push_back(Token{T_RPAREN, ")", line}); break;
                        case '{': tokens.push_back(Token{T_LBRACE, "{", line}); break;
                        case '}': tokens.push_back(Token{T_RBRACE, "}", line}); break;
                        case ';': tokens.push_back(Token{T_SEMICOLON, ";", line}); break;
                        case '>': tokens.push_back(Token{T_GT, ">", line}); break;
                        default:
                        cout << "Unexpected character '" << current << "' at line " << line << endl;
                        exit(1);
                }
                pos++;
            }
            tokens.push_back(Token{T_EOF, "",line});
            return tokens;
        }


        string consumeNumber() {

            size_t start = pos;
            bool hasDecimal = false;

            while (pos < src.size() && (isdigit(src[pos]) || (src[pos] == '.' && !hasDecimal))) {
                if (src[pos] == '.') {
                    hasDecimal = true;
                }
                pos++;
            }

            return src.substr(start, pos - start);
        }

        string consumeWord() {
            size_t start = pos;
            while (pos < src.size() && isalnum(src[pos])) pos++;
            return src.substr(start, pos - start);
        }
};


class Parser {
 

public:
    Parser(const vector<Token> &tokens) {
        this->tokens = tokens;  
        this->pos = 0;   

    }

    void parseProgram() {
        while (tokens[pos].type != T_EOF) {
            parseStatement();
        }
        cout << "Parsing completed successfully! No Syntax Error" << endl;
    }

private:
    vector<Token> tokens;
    size_t pos;

    void syntaxError(const string &message) {
        cout << "Syntax error at line " << tokens[pos].line << ": " << message << endl;
        exit(1);
    }


    void parseStatement() {
        if (tokens[pos].type == T_INT) {
            parseDeclaration();
        }
        else if(tokens[pos].type == T_CONST){
            parseDeclaration();
        }
        else if(tokens[pos].type == T_FLOAT){
            parseDeclaration();
        }
        else if(tokens[pos].type == T_DOUBLE){
            parseDeclaration();
        }
        else if(tokens[pos].type == T_STRING){
            parseDeclaration();
        }
        else if(tokens[pos].type == T_BOOL){
            parseDeclaration();
        }
        else if(tokens[pos].type == T_CHAR){
            parseDeclaration();
        }
        else if (tokens[pos].type == T_ID) {
            parseAssignment();
        } else if (tokens[pos].type == T_IF) {
            parseIfStatement();
        } else if (tokens[pos].type == T_RETURN) {
            parseReturnStatement();
        } else if (tokens[pos].type == T_LBRACE) {  
            parseBlock();
        } else {
            cout << "Syntax error: unexpected token '" << tokens[pos].value << "' at line " << tokens[pos].line << endl;
            exit(1);
        }
    }

    void parseBlock() {
        expect(T_LBRACE);  
        while (tokens[pos].type != T_RBRACE && tokens[pos].type != T_EOF) {
            parseStatement();
        }
        expect(T_RBRACE);  
    }
    void parseDeclaration() {
       if(tokens[pos].type == T_CONST)
       {
            pos++;
            if (tokens[pos].type == T_INT || tokens[pos].type == T_FLOAT || 
            tokens[pos].type == T_DOUBLE || tokens[pos].type == T_STRING || 
            tokens[pos].type == T_BOOL || tokens[pos].type == T_CHAR ) {
            pos++;  
            expect(T_ID);  
            expect(T_SEMICOLON);  
            } else {
                cout << "Syntax error: expected a valid data type at line " << tokens[pos].line << endl;
                exit(1);
            }
       }
        else{
                if (tokens[pos].type == T_INT || tokens[pos].type == T_FLOAT || 
                tokens[pos].type == T_DOUBLE || tokens[pos].type == T_STRING || 
                tokens[pos].type == T_BOOL || tokens[pos].type == T_CHAR ) {
                pos++;  
                expect(T_ID);  
                expect(T_SEMICOLON);  
                } else {
                 cout << "Syntax error: expected a valid data type at line " << tokens[pos].line << endl;
                 exit(1);
                }
        }
    }

    void parseAssignment() {
        expect(T_ID);
        expect(T_ASSIGN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseIfStatement() {
        expect(T_IF);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();  
        if (tokens[pos].type == T_ELSE) {
            expect(T_ELSE);
            parseStatement();  
        }
    }

    void parseReturnStatement() {
        expect(T_RETURN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseExpression() {
        parseTerm();
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS) {
            pos++;
            parseTerm();
        }
        if (tokens[pos].type == T_GT) {
            pos++;
            parseExpression(); 
        }
    }

    void parseTerm() {
        parseFactor();
        while (tokens[pos].type == T_MUL || tokens[pos].type == T_DIV) {
            pos++;
            parseFactor();
        }
    }

    void parseFactor() {
        if (tokens[pos].type == T_NUM || tokens[pos].type == T_ID) {
            pos++;
        } else if (tokens[pos].type == T_LPAREN) {
            expect(T_LPAREN);
            parseExpression();
            expect(T_RPAREN);
        } else {
          cout << "Syntax error: unexpected token '" << tokens[pos].value << "' at line " << tokens[pos].line << endl;
            exit(1);
        }
    }

    void expect(TokenType type) {
        if (tokens[pos].type == type) {
            pos++;
        } else {
            cout << "Syntax error: unexpected token '" << tokens[pos].value << "' at line " << tokens[pos].line << endl;
            exit(1);
        }
    }
};

int main(int argc, char* argv[]) {
   if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }

    ifstream file(argv[1]);

    if (!file) {
        cerr << "Error: Unable to open file " << argv[1] << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();  
    string input = buffer.str();

    file.close();

    cout<<input<<" "<<endl;

    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();
    
    Parser parser(tokens);
    parser.parseProgram();

    return 0;
}
