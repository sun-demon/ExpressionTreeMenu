// variant 5
#include <conio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <unistd.h>

using namespace std;

struct ExpressionTreeException : runtime_error {
    explicit ExpressionTreeException(const string & what_arg = "ExpressionTreeException")
    : runtime_error(what_arg) {}
};
struct PostfixToExpressionTreeException : ExpressionTreeException {
    explicit PostfixToExpressionTreeException(const string & what_arg = "bad postfix expression")
    : ExpressionTreeException("Bad translation postfix expression to expression tree exception: " + what_arg) {}
};
struct InfixToPostfixException : ExpressionTreeException {
    explicit InfixToPostfixException(const string & what_arg = "bad infix expression")
    : ExpressionTreeException("Bad translation infix expression to postfix expression: " + what_arg) {}
};
struct FileNotFoundException : ExpressionTreeException {
    explicit FileNotFoundException(const string & path)
    : ExpressionTreeException("File '" + path + "' does not found: can't be reading or writing") {}
};

struct Node {
    char data;
    Node * left, * right;

    explicit Node(char data, Node * left = nullptr, Node * right = nullptr)
            : data(data), left(left), right(right) {}
};

string slurp(ifstream & fin) {
    ostringstream buf;
    buf << fin.rdbuf();
    return buf.str();
}

int precedence (char c) noexcept {
    if (c == '^')
        return 3;
    else if (c == '/' || c == '*')
        return 2;
    else if (c == '+' || c == '-')
        return 1;
    else
        return 0;
}

bool isOperator(char c) noexcept {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

bool isOperand (char c) noexcept {
    return isdigit(c);
}

string infixToPostfix(const string& infix) {
    stack<char> operators;
    stringstream postfix;
    for (char i : infix) {
        if (isOperand(i))
            postfix << i;
        else if (i == '(')
            operators.push('(');
        else if (i == ')') {
            for (char top = operators.top(); top != '('; top = operators.top()) {
                if (operators.empty())
                    throw InfixToPostfixException("token \'(\' doesn't founded");
                postfix << operators.top();
                operators.pop();
            }
            operators.pop();
        }
        else if (isOperator(i)) {
            while (!operators.empty() && precedence(i) <= precedence(operators.top())) {
                postfix << operators.top();
                operators.pop();
            }
            operators.push(i);
        } else
            throw InfixToPostfixException(string("bad token: \'") + i + "\'");
    }
    while (!operators.empty()) {
        postfix << operators.top();
        operators.pop();
    }
    return postfix.str();
}

string toPrefix(Node * tree) noexcept {
    stringstream result;
    if (tree != nullptr)
        result << tree->data << toPrefix(tree->left) << toPrefix(tree->right);
    return result.str();
}

string toInfix(Node * tree) noexcept {
    stringstream result;
    if (tree != nullptr)
        result << toInfix(tree->left) << tree->data << toInfix(tree->right);
    return result.str();
}

string toPostfix(Node * tree) noexcept {
    stringstream result;
    if (tree != nullptr)
        result << toPostfix(tree->left) << toPostfix(tree->right) << tree->data;
    return result.str();
}

Node * toExpressionTree(const string& postfix) {
    if (postfix.length() == 0)
        return nullptr;
    stack<Node *> nodes;
    for (char c : postfix) {
        if (isOperator(c)) {
            if (nodes.empty())
                throw PostfixToExpressionTreeException();
            Node * right = nodes.top();
            nodes.pop();
            if (nodes.empty())
                throw PostfixToExpressionTreeException();
            Node * left = nodes.top();
            nodes.pop();
            nodes.push(new Node(c, left, right));
        } else
            nodes.push(new Node(c));
    }
    if (nodes.empty())
        throw PostfixToExpressionTreeException();
    return nodes.top();
}

string toString(const string& prefix, Node * node, bool isLeft) noexcept {
    stringstream result;
    if (node != nullptr) {
        result << prefix;
        result << (isLeft ? "|--" : "---" );
        result << node->data << endl;
        result << toString( prefix + (isLeft ? "|   " : "    "), node->left, true);
        result << toString( prefix + (isLeft ? "|   " : "    "), node->right, false);
    }
    return result.str();
}

string toString(Node * node) noexcept {
    return toString("", node, false);
}

string toFullInfoString(Node * tree) noexcept {
    stringstream result;
    if (tree == nullptr)
        return "Tree is empty\n";
    result << "Tree:\n" << toString(tree) << "\n"
           << "Prefix : " << toPrefix(tree) << "\n"
           << "Infix  : " << toInfix(tree) << "\n"
           << "Postfix: " << toPostfix(tree) << "\n";
    return result.str();
}

void clear(Node * &node) {
    if (node != nullptr) {
        clear(node->left);
        clear(node->right);
        delete node;
        node = nullptr;
    }
}

Node * readTree(const string& path) {
    ifstream fin(path);
    if (!fin)
        throw FileNotFoundException(path);
    string infix = slurp(fin);
    fin.close();
    string postfix = infixToPostfix(infix);
    return toExpressionTree(postfix);
}

void writeTree(Node * tree, const string& path) {
    ofstream fout(path);
    if (!fout.is_open()) {
        fout = ofstream("output.txt");
        if (!fout.is_open()) {
            cerr << toFullInfoString(tree) << flush;
            return;
        }
    }
    fout << toFullInfoString(tree) ;
    fout.close();
}

void printTree(Node * tree) noexcept {
    cout << toFullInfoString(tree) << flush;
}

Node * variantTask(const string & input_path, const string & output_path) {
    Node * tree = readTree(input_path);
    writeTree(tree, output_path);
    printTree(tree);
    return tree;
}

void pressAnyKeyForContinue() noexcept {
    cout << "Press any key for continue..." << flush;
    getch();
}

string getCurrentDateTime(const string& s) {
    time_t now = time(nullptr);
    struct tm timeStruct{};
    char  buf[80];
    timeStruct = *localtime(&now);
    if(s == "now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &timeStruct);
    else if(s == "date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &timeStruct);
    return buf;
}

void log(const char * filePath, const string& logMsg) {
    string now = getCurrentDateTime("now");
    ofstream ofs(filePath, std::ios_base::out | std::ios_base::app );
    if (!ofs.is_open())
        ofs = ofstream("errors.txt", std::ios_base::out | std::ios_base::app);
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}

int main(int argc, char * argv[]) {
    if (argc != 4) {
        cerr << "[Utility format]: ExpressionTreeMenu input.txt output.txt error.txt";
        return 1;
    }

    Node * tree = nullptr;
    while (true) {
        try {
            system("cls");
            cout << "1) variant task\n"
                 << "2) read tree\n"
                 << "3) write tree\n"
                 << "4) print tree\n"
                 << "5) clear tree\n"
                 << "0) exit\n"
                 << "Select item >" << flush;
            key_reading:
            char key = char(getch());
            if (key == '0')
                break;
            if (key == '1') {
                system("cls");
                Node * newTree = variantTask(argv[1], argv[2]);
                clear(tree);
                tree = newTree;
                pressAnyKeyForContinue();
            } else if (key == '2') {
                system("cls");
                cout << "Enter filename path with tree: " << flush;
                string path;
                getline(cin, path);
                Node * newTree = readTree(path);
                clear(tree);
                tree = newTree;
            } else if (key == '3') {
                system("cls");
                cout << "Enter file path for tree writing: " << flush;
                string path;
                getline(cin, path);
                writeTree(tree, path);
            } else if (key == '4') {
                system("cls");
                cout << toFullInfoString(tree);
                pressAnyKeyForContinue();
            } else if (key == '5') {
                clear(tree);
            } else
                goto key_reading;
        } catch (const ExpressionTreeException & exc) {
            log(argv[3], exc.what());
            cout << exc.what() << endl;
            pressAnyKeyForContinue();
        }
    }
    clear(tree);
    return 0;
}
