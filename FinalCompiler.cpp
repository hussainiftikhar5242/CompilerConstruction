#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <regex>
#include "pthread.h"
// #include <mutex>
// #include <bits/std_mutex.h>

using namespace std;

// mutex mtx;

enum TokenTypeValue
{
    T_INT,
    T_STRING,
    T_ID,
    T_NUM,
    T_IF,
    T_STRING_LITERAL,
    T_ELSE,
    T_RETURN,
    T_ASSIGN,
    T_PLUS,
    T_MINUS,
    T_FLOAT_LITERAL,
    T_MUL,
    T_DIV,
    T_LPAREN,
    T_EQ,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_SEMICOLON,
    T_GT,
    T_LT,
    T_BOOL_LITERAL,
    T_EOF,
    T_FOR,
    T_BOOL,
    T_FLOAT,
    T_WHILE,
    T_NEQ,
    T_GE,
    T_LE,
    T_DO
};

struct Token
{
    TokenTypeValue type;
    string value;
    size_t lineNo;
};

struct Symbol
{
    string name;
    string type;
    // Default constructor
    Symbol() : name(""), type("") {}

    Symbol(const string name, const string type)
        : name(name), type(type) {}
};

struct SymbolTable
{
    unordered_map<string, Symbol> table;

    void addSymbol(const string &name, const string &type)
    {
        if (table.find(name) != table.end())
        {
            cout << "Error: Redeclaration of symbol '" << name << "'" << endl;
            exit(1);
        }
        table[name] = Symbol(name, type);
    }
    bool hasSymbol(const string &name)
    {
        return table.find(name) != table.end();
    }
    string getVariableType(const string &name)
    {
        if (table.find(name) == table.end())
        {
            throw runtime_error("Semantic error: Variable '" + name + "' is not declared.");
        }
        return table[name].type;
    }

    Symbol getSymbol(const string &name)
    {
        if (table.find(name) == table.end())
        {
            cout << "Error: Symbol '" << name << "' not found" << endl;
            exit(1);
        }
        return table[name];
    }

    void display()
    {
        cout << "\nSymbol Table:\n";
        cout << "Name\tType\n";
        for (auto &entry : table)
        {
            cout << entry.second.name << "\t" << entry.second.type << "\t"
                 << endl;
        }
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

    string generateLabel(const string &base)
    {
        static int labelCount = 0;
        return base + to_string(labelCount++);
    }

    void generateIfGoto(const string &condition, const string &label)
    {
        tac.push_back("if " + condition + " goto " + label);
    }

    void generateAssembly()
    {
        cout << "\nGenerated Assembly Code:" << endl;
        for (const auto &line : tac)
        {
            parseTACtoAssembly(line);
        }
    }

    vector<string> tac;

private:
    void parseTACtoAssembly(const string &line)
    {

        size_t equalPos = line.find("=");
        size_t opPos = line.find_first_of("+-*/", equalPos);

        string result = line.substr(0, equalPos - 1);
        string arg1, arg2, op;

        if (opPos != string::npos)
        {

            op = line.substr(opPos, 1);
            arg1 = line.substr(equalPos + 2, opPos - (equalPos + 2));
            arg2 = line.substr(opPos + 2);

            cout << "MOV AX, " << arg1 << endl;
            if (op == "+")
                cout << "ADD AX, " << arg2 << endl;
            else if (op == "-")
                cout << "SUB AX, " << arg2 << endl;
            else if (op == "*")
                cout << "MUL " << arg2 << endl;
            else if (op == "/")
                cout << "DIV " << arg2 << endl;
            cout << "MOV " << result << ", AX" << endl;
        }
        else
        {

            arg1 = line.substr(equalPos + 2);
            cout << "MOV " << result << ", " << arg1 << endl;
        }
    }
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
                else if (word == "float")
                {
                    tokens.push_back(Token{T_FLOAT, word, this->lineNo});
                }
                else if (word == "bool")
                {
                    tokens.push_back(Token{T_BOOL, word, this->lineNo});
                }
                else if (word == "true" || word == "false")
                    tokens.push_back(Token{T_BOOL_LITERAL, word, lineNo});
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
                else if (word == "do")
                    tokens.push_back(Token{T_DO, word, this->lineNo});
                else if (word == "return")
                    tokens.push_back(Token{T_RETURN, word, this->lineNo});
                else
                    tokens.push_back(Token{T_ID, word, this->lineNo});
                continue;
            }

            if (current == '"')
            {
                // Handle string literals
                tokens.push_back(Token{T_STRING_LITERAL, consumeString(), lineNo});
                continue;
            }

            switch (current)
            {
            case '=':
                if (pos + 1 < src.size() && src[pos + 1] == '=')
                {
                    pos++;
                    tokens.push_back(Token{T_EQ, "==", lineNo});
                }
                else
                {
                    tokens.push_back(Token{T_ASSIGN, "=", lineNo});
                }
                break;
            case '!':
                if (pos + 1 < src.size() && src[pos + 1] == '=')
                {          // Lookahead
                    pos++; // Consume the '='
                    tokens.push_back(Token{T_NEQ, "!=", lineNo});
                }
                else
                {
                    cerr << "Unexpected character '!' at line " << lineNo << "\n";
                    exit(1);
                }
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
                // Check if the next character is '=' for the >= operator
                if (pos + 1 < src.size() && src[pos + 1] == '=')
                {
                    tokens.push_back(Token{T_GE, ">=", this->lineNo}); // Greater than or equal to
                    pos++;                                             // Move past the '=' character
                }
                else
                {
                    tokens.push_back(Token{T_GT, ">", this->lineNo}); // Just '>'
                }
                break;

            case '<':
                // Check if the next character is '=' for the <= operator
                if (pos + 1 < src.size() && src[pos + 1] == '=')
                {
                    tokens.push_back(Token{T_LE, "<=", this->lineNo}); // Less than or equal to
                    pos++;                                             // Move past the '=' character
                }
                else
                {
                    tokens.push_back(Token{T_LT, "<", this->lineNo}); // Just '<'
                }
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
        while (pos < src.size() && (isdigit(src[pos]) || src[pos] == '.'))
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

    string consumeString()
    {
        size_t start = pos;
        if (src[pos] != '"') // Check if it's not a double quote
        {
            cerr << "Error: String literal should start with a double quote at line " << lineNo << "\n";
            exit(1); // Exit on error
        }
        pos++;
        while (pos < src.size() && src[pos] != '"')
            pos++;
        if (pos >= src.size())
        {
            cerr << "Error: Unterminated string literal\n";
            exit(1);
        }
        pos++;
        return src.substr(start, pos - start);
    }
};

class Parser
{
    string currentScope = "global";

public:
    Parser(const vector<Token> &tokens, TACGenerator &tacGen) : tacGen(tacGen)
    {
        this->tokens = tokens;
        this->pos = 0;
        tokenMap[T_INT] = "int";
        tokenMap[T_ID] = "identifier";
        tokenMap[T_NUM] = "number";
        tokenMap[T_FLOAT] = "float";
        tokenMap[T_IF] = "if";
        tokenMap[T_ELSE] = "else";
        tokenMap[T_RETURN] = "return";
        tokenMap[T_ASSIGN] = "=";
        tokenMap[T_PLUS] = "+";
        tokenMap[T_MINUS] = "-";
        tokenMap[T_EQ] = "==";
        tokenMap[T_NEQ] = "!=";
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
        tokenMap[T_BOOL] = "bool";
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
        symbolTable.display();
        tacGen.printTAC();
        tacGen.generateAssembly();
    }

private:
    vector<Token> tokens;
    size_t pos;
    SymbolTable symbolTable;
    TACGenerator &tacGen;

    unordered_map<int, string> tokenMap;
    int tempCount = 0;

    void parseStatement()
    {
        if (tokens[pos].type == T_INT)
        {
            parseDeclaration();
        }
        else if (tokens[pos].type == T_FLOAT)
        {
            parseDeclaration();
        }
        else if (tokens[pos].type == T_STRING)
        {
            parseDeclaration();
        }
        else if (tokens[pos].type == T_BOOL)
        {
            parseDeclaration();
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

        else if (tokens[pos].type == T_DO)
        {
            parseDoWhileLoop();
        }

        else
        {
            cout << "Syntax error: unexpected token " << tokens[pos].value << endl;
            exit(1);
        }
    }

    string generateLabel(const string &base)
    {
        static int labelCount = 0;
        return base + to_string(labelCount++);
    }

    void parseDoWhileLoop()
    {
        expect(T_DO); // Expect 'do'

        string startLabel = generateLabel("L");
        tacGen.generateLabel(startLabel); // Generate start label

        expect(T_LBRACE); // Expect '{'
        parseBlock();     // Parse the block
        expect(T_RBRACE); // Expect '}'

        expect(T_WHILE);  // Expect 'while'
        expect(T_LPAREN); // Expect '(' for condition

        string condition = parseCondition(); // Parse the condition
        expect(T_RPAREN);                    // Expect ')' after condition
        expect(T_SEMICOLON);                 // Expect ';'

        tacGen.generateIfGoto(condition, startLabel); // Generate TAC to loop back
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
        Token typeToken = tokens[pos++];
        Token idToken = tokens[pos++];

        if (idToken.type != T_ID)
        {
            cerr << "Syntax error: Expected identifier\n";
            exit(1);
        }

        if (symbolTable.hasSymbol(idToken.value))
        {
            cerr << "Error: Variable '" << idToken.value << "' already declared.\n";
            exit(1);
        }

        symbolTable.addSymbol(idToken.value, typeToken.value);
        if (tokens[pos].type == T_ASSIGN)
        {
            pos++; // Consume '='
            string value = parseExpression();
            checkUndeclaredVariable(value);
            tacGen.generateAssign(idToken.value, value);
        }

        if (tokens[pos].type != T_SEMICOLON)
        {
            cerr << "Syntax error: Expected ';' after declaration.\n";
            exit(1);
        }

        expect(T_SEMICOLON);
    }
    void checkUndeclaredVariable(const string &expr)
    {
        // If the expression is a variable (ID), check if it is declared
        if (isVariable(expr) && !symbolTable.hasSymbol(expr))
        {
            cerr << "Error: Variable '" << expr << "' used but not declared.\n";
            exit(1);
        }
    }

    // Helper function to check if a string is a variable (ID)
    bool isVariable(const string &str)
    {
        return isalpha(str[0]) || str[0] == '_'; // Variable names typically start with a letter or underscore
    }

    void parseAssignment()
    {
        string varName = tokens[pos++].value;

        if (!symbolTable.hasSymbol(varName))
        {
            cerr << "Error: Variable '" << varName << "' not declared.\n";
            exit(1);
        }

        expect(T_ASSIGN);

        string value = parseExpression();
        string varType = symbolTable.getVariableType(varName);

        if (!isCompatibleType(varType, value))
        {
            cerr << "Type error: Cannot assign value '" << value
                 << "' to variable of type '" << varType
                 << "'\n";
            exit(1);
        }

        tacGen.generateAssign(varName, value);
        expect(T_SEMICOLON);
    }

    bool isCompatibleType(const string &varType, const string &value)
    {
        if (!value.empty() && value[0] == 't')
            return true;
        if (varType == "int" && isInteger(value))
            return true;
        if (varType == "float" && (isInteger(value) || isFloat(value)))
            return true;
        if (varType == "bool" && (value == "true" || value == "false"))
            return true;
        if (varType == "string" && isStringLiteral(value))
            return true;
        return false;
    }

    bool isInteger(const string &value)
    {
        return !value.empty() && all_of(value.begin(), value.end(), ::isdigit);
    }

    bool isFloat(const string &value)
    {
        bool hasDecimal = false;
        for (char c : value)
        {
            if (c == '.')
            {
                if (hasDecimal)
                    return false; // Multiple decimals
                hasDecimal = true;
            }
            else if (!isdigit(c))
            {
                return false; // Non-numeric characters
            }
        }
        return hasDecimal; // Must contain one decimal point
    }
    bool isStringLiteral(const string &value)
    {
        return value.size() >= 2 && value.front() == '"' && value.back() == '"';
    }

    void parseForLoop()
    {
        expect(T_FOR);
        expect(T_LPAREN);
        parseAssignment();
        string condition = parseExpression();
        expect(T_SEMICOLON);
        string increment = parseIncrement();
        expect(T_RPAREN);
        parseBlock();
    }

    string parseIncrement()
    {
        string varName = tokens[pos].value;
        expect(T_ID);

        if (tokens[pos].type == T_ASSIGN)
        {
            expect(T_ASSIGN);
            string value = parseExpression();
            tacGen.generateAssign(varName, value);
        }
        else if (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS)
        {
            string op = tokens[pos].value;
            pos++;
            if (tokens[pos].type == T_NUM)
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
        string condition = parseCondition();
        expect(T_RPAREN);
        parseStatement();
        if (tokens[pos].type == T_ELSE)
        {
            expect(T_ELSE);
            parseStatement();
        }
    }

    string parseCondition()
    {
        string left = parseExpression();

        // Check for the supported relational operators
        if (tokens[pos].type == T_GT || tokens[pos].type == T_LT ||
            tokens[pos].type == T_ASSIGN || tokens[pos].type == T_EQ ||
            tokens[pos].type == T_LE || tokens[pos].type == T_GE ||
            tokens[pos].type == T_NEQ)
        {
            string op = tokens[pos++].value;        // Consume the operator
            string right = parseExpression();       // Parse the right-hand side expression
            string temp = generateTemp();           // Generate a temporary variable for the result
            tacGen.generate(op, left, right, temp); // Generate the TAC for the comparison
            return temp;                            // Return the temporary result variable
        }

        return left; // Return the left expression if no relational operator is found
    }

    string generateTemp()
    {
        return "t" + to_string(tempCount++);
    }

    void parseReturnStatement()
    {
        expect(T_RETURN);
        string returnValue = parseExpression();
        tacGen.generateAssign("return_value", returnValue);
        expect(T_SEMICOLON);
    }

    // string parseExpression()
    // {
    //     string result = parseRelational();
    //     while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS)
    //     {
    //         string op = tokens[pos].value;
    //         pos++;
    //         string arg2 = parseRelational();
    //         string tempVar = "t" + to_string(tempCount++);
    //         tacGen.generate(op, result, arg2, tempVar);
    //         result = tempVar;
    //     }
    //     return result;
    // }

    string parseExpression()
    {
        string result = parseRelational();
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS)
        {
            string op = tokens[pos].value;
            pos++;
            string arg2 = parseRelational();

            if (isConstant(result) && isConstant(arg2))
            {
                result = performConstantFolding(result, arg2, op);
            }
            else
            {
                string tempVar = "t" + to_string(tempCount++);
                tacGen.generate(op, result, arg2, tempVar);
                result = tempVar;
            }
        }
        return result;
    }

    bool isConstant(const string &value)
    {
        return isInteger(value) || isFloat(value);
    }

    string performConstantFolding(const string &left, const string &right, const string &op)
    {
        bool leftIsInt = isInteger(left);
        bool rightIsInt = isInteger(right);
        double leftVal = isInteger(left) ? stod(left) : stod(left);
        double rightVal = isInteger(right) ? stod(right) : stod(right);
        double resultVal = 0.0;

        if (op == "+")
            resultVal = leftVal + rightVal;
        else if (op == "-")
            resultVal = leftVal - rightVal;
        else if (op == "*")
            resultVal = leftVal * rightVal;
        else if (op == "/")
            resultVal = leftVal / rightVal;

        // Return the computed result as a string, while maintaining the type precision
        // Convert to string with the appropriate type handling:
        if (leftIsInt && rightIsInt && resultVal == static_cast<int>(resultVal))
        {
            // If the result is effectively an integer, return it as an integer
            return to_string(static_cast<int>(resultVal));
        }
        else
        {
            // Otherwise, return as a float string
            return to_string(resultVal);
        }
    }

    string parseRelational()
    {
        string result = parseTerm();

        while (tokens[pos].type == T_GT || tokens[pos].type == T_LT || tokens[pos].type == T_EQ ||
               tokens[pos].type == T_NEQ || tokens[pos].type == T_LE || tokens[pos].type == T_GE)
        {
            string op = tokens[pos].value;
            pos++;
            string arg2 = parseTerm();

            if (isConstant(result) && isConstant(arg2))
            {
                result = performConstantFolding(result, arg2, op);
            }
            else
            {
                string tempVar = "t" + to_string(tempCount++);
                tacGen.generate(op, result, arg2, tempVar);
                result = tempVar;
            }
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

            if (isConstant(result) && isConstant(arg2))
            {
                result = performConstantFolding(result, arg2, op);
            }
            else
            {
                string tempVar = "t" + to_string(tempCount++);
                tacGen.generate(op, result, arg2, tempVar);
                result = tempVar;
            }
        }
        return result;
    }

    string parseFactor()
    {
        if (tokens[pos].type == T_NUM || tokens[pos].type == T_FLOAT_LITERAL || tokens[pos].type == T_BOOL_LITERAL || tokens[pos].type == T_STRING_LITERAL || tokens[pos].type == T_ID)
        {
            return tokens[pos++].value;
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

    void expect(TokenTypeValue type)
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
void *lexerThread(void *arg)
{
    cout << "Lexer thread started" << endl;
    Lexer *lexer = (Lexer *)arg;
    lexer->tokenize();
    cout << "Lexer thread finished" << endl;
    cout << "---------------------------" << endl;
    pthread_exit(NULL);
}

void *parserThread(void *arg)
{
    cout << "Parser thread started" << endl;
    Parser *parser = (Parser *)arg;
    parser->parseProgram();
    cout << "Parser thread finished" << endl;
    cout << "---------------------------" << endl;

    pthread_exit(NULL);
}

int main()
{
    string input2 = R"(
        int x = 10;
        int a;
        int w;
        w = 455;
        if(x > 0){
            a = 0;
        }
        else if(x > 10){
            a = 1;
        }
    )";

    string input = R"( 
        /*Hello brother how are u
        dhsaudhuiahweiofwioe
        vsdviohviowre
        vrovpwro*/
        int a;
        //HEllo
        a = 6;
        int b;
        b = a + 10;
        string name;
        float c;
        bool flag;
        name = "John";
        flag = true;
        c = 5.2 * 3;
        int g;
        g = 7 + 1;
        a = a / 1;
        if (b < a) {
            return b;
        } else {
            return 0;
        }
        int x;
        if (x > 5) {
            x = x + 1;
        } else if(x != 5) {
            x = x - 1;
        } else {
            x = 0;
        }
        int y;
        y = 10;
        for (x = 0; x < 5; x = x + 1)
        {
            int abc;
            abc = abc + 1;
            while (x < 0)
            {
                x = x - 1;
            }
        }
        while (y < 10)
        {
            y = y + 1;
        }
        return y + 1;
    )";

    Lexer lexer1(input);
    Lexer lexer2(input2);

    vector<Token> tokens1 = lexer1.tokenize();
    vector<Token> tokens2 = lexer2.tokenize();

    TACGenerator tacGen1, tacGen2;
    Parser parser1(tokens1, tacGen1);
    Parser parser2(tokens2, tacGen2);

    pthread_t lexerTid1, lexerTid2, parserTid1, parserTid2;

    pthread_create(&lexerTid1, NULL, lexerThread, &lexer1);
    pthread_create(&lexerTid2, NULL, lexerThread, &lexer2);

    // Wait for lexer threads to finish
    pthread_join(lexerTid1, NULL);
    pthread_join(lexerTid2, NULL);

    // Create parser threads for both tokenized inputs
    pthread_create(&parserTid1, NULL, parserThread, &parser1);
    pthread_create(&parserTid2, NULL, parserThread, &parser2);

    // Wait for parser threads to finish
    pthread_join(parserTid1, NULL);
    pthread_join(parserTid2, NULL);

    return 0;
}
