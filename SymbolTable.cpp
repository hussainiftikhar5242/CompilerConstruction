#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <map>
#include <unordered_map>

using namespace std;

enum TokenType {
    T_INT, T_ID, T_NUM, T_AGAR, T_ELSE, T_WAPIS,
    T_ASSIGN, T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE, T_FLOAT, T_STRING, T_ERROR, T_CHAR, T_DOUBLE, T_BOOL,
    T_CONST, T_SEMICOLON, T_GT, T_EOF,
    T_WHILE, T_AND, T_EQ, T_NEQ,
    T_BACKSLASH, T_FOR,
};

struct Token {
    TokenType type;
    string value;
    int line;
};

struct Symbol {
    string name;
    string type;
    string scope;
    bool isConst;

    // Default constructor
    Symbol() : name(""), type(""), scope(""), isConst(false) {}

    Symbol(string name, string type, string scope, bool isConst)
        : name(name), type(type), scope(scope), isConst(isConst) {}
};

class SymbolTable {
private:
    unordered_map<string, Symbol> table;

public:
    void addSymbol(const string& name, const string& type, const string& scope, bool isConst) {
        if (table.find(name) != table.end()) {
            cout << "Error: Redeclaration of symbol '" << name << "'" << endl;
            exit(1);
        }
        table[name] = Symbol(name, type, scope, isConst);
    }

    bool hasSymbol(const string& name) {
        return table.find(name) != table.end();
    }

    Symbol getSymbol(const string& name) {
        if (table.find(name) == table.end()) {
            cout << "Error: Symbol '" << name << "' not found" << endl;
            exit(1);
        }
        return table[name];
    }

    void display() {
        cout << "\nSymbol Table:\n";
        cout << "Name\tType\tScope\tConst\n";
        for (auto& entry : table) {
            cout << entry.second.name << "\t" << entry.second.type << "\t" 
                 << entry.second.scope << "\t" << (entry.second.isConst ? "Yes" : "No") << endl;
        }
    }
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
                    else if (word == "agar") tokens.push_back(Token{T_AGAR, word, line});
                    else if (word == "else") tokens.push_back(Token{T_ELSE, word, line});
                    else if (word == "wapis") tokens.push_back(Token{T_WAPIS, word, line});
                    else if (word == "float") tokens.push_back(Token{T_FLOAT, word, line});
                    else if (word == "string") tokens.push_back(Token{T_STRING, word, line});
                    else if (word == "char") tokens.push_back(Token{T_CHAR, word, line});
                    else if (word == "double") tokens.push_back(Token{T_DOUBLE, word, line});
                    else if (word == "bool") tokens.push_back(Token{T_BOOL, word, line});
                    else if (word == "const") tokens.push_back(Token{T_CONST, word, line});
                    else if (word == "while") tokens.push_back(Token{T_WHILE, word, line});
                    else if (word == "for") tokens.push_back(Token{T_FOR, word, line});
                    
                    else tokens.push_back(Token{T_ID, word, line});
                    continue;
                }
                
                switch (current) {
                        
                        case '&':
                            if (src[pos + 1] == '&') {
                                tokens.push_back(Token{T_AND, "&&", line});
                                pos++;
                            }
                        break;
                        case '=':
                            if (src[pos + 1] == '=') {
                                tokens.push_back(Token{T_EQ, "==", line});
                                pos++;
                            } else {
                                tokens.push_back(Token{T_ASSIGN, "=", line});
                            }
                        break;
                        case '!':
                            if (src[pos + 1] == '=') {
                                tokens.push_back(Token{T_NEQ, "!=", line});
                                pos++;
                            } else {
                                cout << "Unexpected character '" << current << "' at line " << line << endl;
                                exit(1);
                            }
                        break;
                        case '+': tokens.push_back(Token{T_PLUS, "+", line}); break;
                        case '-': tokens.push_back(Token{T_MINUS, "-", line}); break;
                        case '*': tokens.push_back(Token{T_MUL, "*", line}); break;
                        case '/': 
                           if (src[pos + 1] == '/') {
                            consumeSingleLineComment();
                           }
                            else{
                                tokens.push_back(Token{T_DIV, "/", line});
                            }
                            break;
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

         void consumeSingleLineComment() {
            pos += 2;  
            while (pos < src.size() && src[pos] != '\n') {
                pos++;
            }
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
private:
    vector<Token> tokens;
    size_t pos;
    SymbolTable symbolTable;
    string currentScope = "global";

    void syntaxError(const string& message) {
        cout << "Syntax error at line " << tokens[pos].line << ": " << message << endl;
        exit(1);
    }

    void parseStatement() {
        if (tokens[pos].type == T_INT || tokens[pos].type == T_FLOAT ||
            tokens[pos].type == T_DOUBLE || tokens[pos].type == T_STRING ||
            tokens[pos].type == T_BOOL || tokens[pos].type == T_CHAR) {
            parseDeclaration();
        } else if (tokens[pos].type == T_CONST) {
            parseDeclaration(true);
        } else if (tokens[pos].type == T_ID) {
            parseAssignment();
        } else if (tokens[pos].type == T_AGAR) {
            parseIfStatement();
        } else if (tokens[pos].type == T_WAPIS) {
            parseReturnStatement();
        } else if (tokens[pos].type == T_LBRACE) {
            parseBlock();
        } else if (tokens[pos].type == T_WHILE) {
            parseWhileStatement();
        } else if (tokens[pos].type == T_FOR) {
            parseForStatement();
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

    void parseIfStatement() {
        expect(T_AGAR);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();  
        if (tokens[pos].type == T_ELSE) {
            expect(T_ELSE);
            parseStatement();  
        }
    }

    void parseDeclaration(bool isConst = false) {
        string type = tokens[pos].value;
        pos++; // Skip the type
        string name = tokens[pos].value;
        pos++; // Skip the identifier

        if (symbolTable.hasSymbol(name)) {
            cout << "Error: Variable '" << name << "' already declared" << endl;
            exit(1);
        }

        symbolTable.addSymbol(name, type, currentScope, isConst);
        expect(T_SEMICOLON);
    }

    void parseWhileStatement() {
        expect(T_WHILE);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();
    }

    void parseForStatement() {
        expect(T_FOR);
        expect(T_LPAREN);
        parseDeclaration();
        parseExpression();
        expect(T_SEMICOLON);
        expect(T_RPAREN);
        parseStatement();
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


    void parseAssignment() {
        string name = tokens[pos].value;
        if (!symbolTable.hasSymbol(name)) {
            cout << "Error: Variable '" << name << "' not declared" << endl;
            exit(1);
        }

        Symbol symbol = symbolTable.getSymbol(name);
        if (symbol.isConst) {
            cout << "Error: Cannot assign to constant variable '" << name << "'" << endl;
            exit(1);
        }

        pos++; // Skip identifier
        expect(T_ASSIGN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void expect(TokenType type) {
        if (tokens[pos].type == type) {
            pos++;
        } else {
            cout << "Syntax error: unexpected token '" << tokens[pos].value << "' at line " << tokens[pos].line << endl;
            exit(1);
        }
    }

    
    void parseReturnStatement() {
        expect(T_WAPIS);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseExpression() {
        parseTerm();
        
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS || 
            tokens[pos].type == T_GT || tokens[pos].type == T_EQ || 
            tokens[pos].type == T_NEQ || tokens[pos].type == T_AND ) {
            
            pos++; 
            parseTerm();  
            
            
        }
    }
    // Other methods (parseExpression, parseTerm, etc.) remain unchanged.

public:
    Parser(const vector<Token>& tokens) : tokens(tokens), pos(0) {}

    void parseProgram() {
        while (tokens[pos].type != T_EOF) {
            parseStatement();
        }
        cout << "Parsing completed successfully! No Syntax Error" << endl;
        symbolTable.display();
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

    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    parser.parseProgram();

    return 0;
}
