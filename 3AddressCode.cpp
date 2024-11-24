#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <map>
#include <unordered_map>

using namespace std;

enum TokenType
{
    T_INT,
    T_STRING,
    T_ID,
    T_NUM,
    T_IF,
    T_ELSE,
    T_RETURN,
    T_ASSIGN,
    T_PLUS,
    T_MINUS,
    T_MUL,
    T_DIV,
    T_LPAREN,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_SEMICOLON,
    T_GT,
    T_LT,
    T_EOF,
    T_FOR,
    T_WHILE
};

struct Token
{
    TokenType type;
    string value;
    size_t lineNo;
};

struct SymbolEntry
{
    TokenType dataType;
    string value;
};

struct SymbolTable
{
    unordered_map<string, SymbolEntry> table;

    bool add(const string &name, const SymbolEntry &entry)
    {
        if (table.find(name) != table.end())
        {
            return false;
        }
        table[name] = entry;
        return true;
    }
};

class TACGenerator
{
public:
    void generate(const string &op, const string &arg1, const string &arg2, const string &result)
    {
        tac.push_back(result + " = " + arg1 + " " + op + " " + arg2);
    }

    void generateAssign(const string &var, const string &value)
    {
        tac.push_back(var + " = " + value);
    }

    void printTAC()
    {
        cout << "Three Address Code:" << endl;
        for (const auto &line : tac)
        {
            cout << line << endl;
        }
    }

private:
    vector<string> tac;
};

class Lexer
{
private:
    string src;
    size_t pos;
    size_t lineNo;

public:
    Lexer(const string &src)
    {
        this->src = src;
        this->pos = 0;
        this->lineNo = 0;
    }

    vector<Token> tokenize()
    {
        vector<Token> tokens;
        while (pos < src.size())
        {
            char current = src[pos];
            if (current == '/')
            {
                if (pos + 1 < src.size())
                {
                    if (src[pos + 1] == '/')
                    {
                        while (current != '\n')
                        {
                            pos++;
                            current = src[pos];
                        }
                        this->lineNo++;
                        pos++;
                        continue;
                    }
                    else if (src[pos + 1] == '*')
                    {
                        while (true)
                        {
                            pos++;
                            current = src[pos];
                            if (current == '\n')
                            {
                                this->lineNo++;
                            }
                            else if (current == '*')
                            {
                                if (pos + 1 < src.size())
                                {
                                    if (src[pos + 1] == '/')
                                    {
                                        pos++;
                                        current = src[pos];
                                        if (pos + 1 < src.size())
                                        {
                                            pos++;
                                            current = src[pos];
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (current == '\n')
            {
                this->lineNo++;
                pos++;
                continue;
            }
            if (isspace(current))
            {
                pos++;
                continue;
            }
            if (isdigit(current))
            {
                tokens.push_back(Token{T_NUM, consumeNumber(), this->lineNo});
                continue;
            }
            if (isalpha(current))
            {
                string word = consumeWord();
                if (word == "int")
                {
                    tokens.push_back(Token{T_INT, word, this->lineNo});
                }
                else if (word == "if")
                    tokens.push_back(Token{T_IF, word, this->lineNo});
                else if (word == "string")
                    tokens.push_back(Token{T_STRING, word, this->lineNo});
                else if (word == "else")
                    tokens.push_back(Token{T_ELSE, word, this->lineNo});
                else if (word == "for")
                    tokens.push_back(Token{T_FOR, word, this->lineNo});
                else if (word == "while")
                    tokens.push_back(Token{T_WHILE, word, this->lineNo});
                else if (word == "return")
                    tokens.push_back(Token{T_RETURN, word, this->lineNo});
                else
                    tokens.push_back(Token{T_ID, word, this->lineNo});
                continue;
            }

            switch (current)
            {
            case '=':
                tokens.push_back(Token{T_ASSIGN, "=", this->lineNo});
                break;
            case '+':
                tokens.push_back(Token{T_PLUS, "+", this->lineNo});
                break;
            case '-':
                tokens.push_back(Token{T_MINUS, "-", this->lineNo});
                break;
            case '*':
                tokens.push_back(Token{T_MUL, "*", this->lineNo});
                break;
            case '/':
                tokens.push_back(Token{T_DIV, "/", this->lineNo});
                break;
            case '(':
                tokens.push_back(Token{T_LPAREN, "(", this->lineNo});
                break;
            case ')':
                tokens.push_back(Token{T_RPAREN, ")", this->lineNo});
                break;
            case '{':
                tokens.push_back(Token{T_LBRACE, "{", this->lineNo});
                break;
            case '}':
                tokens.push_back(Token{T_RBRACE, "}", this->lineNo});
                break;
            case ';':
                tokens.push_back(Token{T_SEMICOLON, ";", this->lineNo});
                break;
            case '>':
                tokens.push_back(Token{T_GT, ">", this->lineNo});
                break;
            case '<':
                tokens.push_back(Token{T_LT, "<", this->lineNo});
                break;
            default:
                cout << "Unexpected character: " << current << endl;
                exit(1);
            }
            pos++;
        }
        tokens.push_back(Token{T_EOF, "", this->lineNo});
        return tokens;
    }

    string consumeNumber()
    {
        size_t start = pos;
        while (pos < src.size() && isdigit(src[pos]))
            pos++;
        return src.substr(start, pos - start);
    }

    string consumeWord()
    {
        size_t start = pos;
        while (pos < src.size() && isalnum(src[pos]))
        {
            pos++;
        }
        return src.substr(start, pos - start);
    }
};

class Parser
{
public:
    Parser(const vector<Token> &tokens, TACGenerator &tacGen) : tacGen(tacGen)
    {
        this->tokens = tokens;
        this->pos = 0;
        tokenMap[T_INT] = "int";
        tokenMap[T_ID] = "identifier";
        tokenMap[T_NUM] = "number";
        tokenMap[T_IF] = "if";
        tokenMap[T_ELSE] = "else";
        tokenMap[T_RETURN] = "return";
        tokenMap[T_ASSIGN] = "=";
        tokenMap[T_PLUS] = "+";
        tokenMap[T_MINUS] = "-";
        tokenMap[T_MUL] = "*";
        tokenMap[T_DIV] = "/";
        tokenMap[T_LPAREN] = "(";
        tokenMap[T_RPAREN] = ")";
        tokenMap[T_LBRACE] = "{";
        tokenMap[T_RBRACE] = "}";
        tokenMap[T_SEMICOLON] = ";";
        tokenMap[T_GT] = ">";
        tokenMap[T_LT] = "<";
        tokenMap[T_EOF] = "end of file";
        tokenMap[T_FOR] = "for";
        tokenMap[T_WHILE] = "while";
        tokenMap[T_STRING] = "string";
    }

    void parseProgram()
    {
        while (tokens[pos].type != T_EOF)
        {
            parseStatement();
        }
        cout << "Parsing completed successfully! No Syntax Error" << endl;
        tacGen.printTAC();
    }

private:
    vector<Token> tokens;
    size_t pos;
    SymbolTable symbolTable;
    TACGenerator &tacGen;

    unordered_map<int, string> tokenMap;
    int tempCount = 0; // Counter for temporary variable names

    void parseStatement()
    {
        if (tokens[pos].type == T_INT)
        {
            parseDeclaration();
        }
        else if (tokens[pos].type == T_STRING)
        {
            parseStringDeclaration();
        }
        else if (tokens[pos].type == T_FOR)
        {
            parseForLoop();
        }
        else if (tokens[pos].type == T_WHILE)
        {
            parseWhileLoop();
        }
        else if (tokens[pos].type == T_ID)
        {
            parseAssignment();
        }
        else if (tokens[pos].type == T_IF)
        {
            parseIfStatement();
        }
        else if (tokens[pos].type == T_RETURN)
        {
            parseReturnStatement();
        }
        else if (tokens[pos].type == T_LBRACE)
        {
            parseBlock();
        }
        else
        {
            cout << "Syntax error: unexpected token " << tokens[pos].value << endl;
            exit(1);
        }
    }

    void parseBlock()
    {
        expect(T_LBRACE);
        while (tokens[pos].type != T_RBRACE && tokens[pos].type != T_EOF)
        {
            parseStatement();
        }
        expect(T_RBRACE);
    }

    void parseDeclaration()
    {
        expect(T_INT);
        string varName = tokens[pos].value;
        expect(T_ID);
        SymbolEntry entry = {T_INT, ""};
        if (!symbolTable.add(varName, entry))
        {
            cout << "Error: Variable " << varName << " already declared!" << endl;
            exit(1);
        }
        expect(T_SEMICOLON);
    }

    void parseStringDeclaration()
    {
        expect(T_STRING);
        expect(T_ID);
        expect(T_SEMICOLON);
    }

    void parseAssignment()
    {
        string varName = tokens[pos].value;
        expect(T_ID);
        if (symbolTable.table.find(varName) == symbolTable.table.end())
        {
            cout << "Error: Variable " << varName << " not declared!" << endl;
            exit(1);
        }
        expect(T_ASSIGN);
        string value = parseExpression();
        tacGen.generateAssign(varName, value);
        expect(T_SEMICOLON);
    }

void parseForLoop()
{
    expect(T_FOR);
    expect(T_LPAREN);
    parseAssignment(); // Initialization
    string condition = parseExpression(); // Condition
    expect(T_SEMICOLON);
    string increment = parseIncrement(); // Increment/Decrement
    expect(T_RPAREN);
    parseBlock();
}

string parseIncrement()
{
    // This function can handle both simple increments (x++) and complex expressions (x = x + 1)
    string varName = tokens[pos].value;
    expect(T_ID);

    if (tokens[pos].type == T_ASSIGN) // Check if it's an assignment
    {
        expect(T_ASSIGN);
        string value = parseExpression(); // Parse the expression on the right side
        tacGen.generateAssign(varName, value);
    }
    else if (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS) // Handle simple increments
    {
        string op = tokens[pos].value;
        pos++;
        if (tokens[pos].type == T_NUM) // Expecting a number after the operator
        {
            string incrementAmount = tokens[pos].value;
            expect(T_NUM);
            string tempVar = "t" + to_string(tempCount++);
            tacGen.generate(op, varName, incrementAmount, tempVar);
            tacGen.generateAssign(varName, tempVar);
        }
    }
    else
    {
        cout << "Syntax error: expected increment expression but found " << tokens[pos].value << endl;
        exit(1);
    }
    
    return varName; // Return the incremented variable
}
    void parseWhileLoop()
    {
        expect(T_WHILE);
        expect(T_LPAREN);
        string condition = parseExpression(); // Condition
        expect(T_RPAREN);
        parseBlock();
    }

    void parseIfStatement()
    {
        expect(T_IF);
        expect(T_LPAREN);
        string condition = parseExpression(); // Condition
        expect(T_RPAREN);
        parseStatement();
        if (tokens[pos].type == T_ELSE)
        {
            expect(T_ELSE);
            parseStatement();
        }
    }

    void parseReturnStatement()
    {
        expect(T_RETURN);
        string returnValue = parseExpression();
        tacGen.generateAssign("return_value", returnValue);
        expect(T_SEMICOLON);
    }

    string parseExpression()
    {
        string result = parseRelational(); // Start with relational expressions
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS)
        {
            string op = tokens[pos].value;
            pos++;
            string arg2 = parseRelational();
            string tempVar = "t" + to_string(tempCount++);
            tacGen.generate(op, result, arg2, tempVar);
            result = tempVar;
        }
        return result;
    }

    string parseRelational()
    {
        string result = parseTerm();
        while (tokens[pos].type == T_GT || tokens[pos].type == T_LT)
        {
            string op = tokens[pos].value;
            pos++;
            string arg2 = parseTerm();
            string tempVar = "t" + to_string(tempCount++);
            tacGen.generate(op, result, arg2, tempVar);
            result = tempVar;
        }
        return result;
    }

    string parseTerm()
    {
        string result = parseFactor();
        while (tokens[pos].type == T_MUL || tokens[pos].type == T_DIV)
        {
            string op = tokens[pos].value;
            pos++;
            string arg2 = parseFactor();
            string tempVar = "t" + to_string(tempCount++);
            tacGen.generate(op, result, arg2, tempVar);
            result = tempVar;
        }
        return result;
    }

    string parseFactor()
    {
        if (tokens[pos].type == T_NUM || tokens[pos].type == T_ID)
        {
            string value = tokens[pos].value;
            pos++;
            return value;
        }
        else if (tokens[pos].type == T_LPAREN)
        {
            expect(T_LPAREN);
            string result = parseExpression();
            expect(T_RPAREN);
            return result;
        }
        else
        {
            cout << "Syntax error: unexpected token " << tokens[pos].value << endl;
            exit(1);
        }
    }


    void expect(TokenType type)
    {
        if (tokens[pos].type == type)
        {
            pos++;
        }
        else
        {
            cout << "Syntax error: expected " << tokenMap[type] << " but found " << tokens[pos].value << " on line no: " << tokens[pos].lineNo << endl;
            exit(1);
        }
    }
};

int main()
{
    string input = R"(
    /*Hello brother how are u
    dhsaudhuiahweiofwioe
    vsdviohviowre
    vrovpwro*/
        int a;
        //HEllo
        a = 5;
        a = a + 1;
        int b;
        b = a + 10;
        if (b < a) {
            return b;
        } else {
            return 0;
        }
        int x;
        int y;
        y = 10;
        for (x = 0; x < b; x = x+1)
        {
            int abc;
            abc = abc + 1;
            while (x < 0)
            {
                x = x - 1;
            }
        }
        return y + 1;
    )";

    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();

    TACGenerator tacGen;
    Parser parser(tokens, tacGen);
    parser.parseProgram();

    return 0;
}