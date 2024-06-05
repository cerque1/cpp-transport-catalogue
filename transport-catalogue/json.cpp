#include <cmath>

#include "json.h"
#include <string.h>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    bool is_end = false;

    for (char c; input >> c;) {
        if(c == ']'){
            is_end = true;
            break;
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if(!is_end){
        throw ParsingError("Array is not closed");
    }
    return Node(move(result));
}

Node LoadNum(istream& input) {
    char c;
    string result;
    bool is_double = false;

    while (input >> c) {
        if(isdigit(c)){
            result += c;
        }
        else if(c == '.'){
            result += '.';
            is_double = true;
        }
        else if(c == '-'){
            result = '-' + result;
        }
        else if(tolower(c) == 'e'){
            is_double = true;
            result += 'e';
            while(input >> c){
                if(isdigit(c)){
                    result += c;
                }
                else if(c == '-'){
                    result += '-';
                }
                else{
                    break;
                }
            }
        }
        else{
            input.putback(c);
            break;
        }
    }
    try{
        if(is_double){
            try{
                return Node(stod(result));
            }catch(...){}
        }
        return Node(stoi(result));
    } catch(...){
        throw ParsingError("Parsing num error");
    }
}

Node LoadString(std::istream& input) {
    string line; 

    auto pos = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();

    while(true){
        if(pos == end){
            throw ParsingError("string is not closed");
        }

        char ch = *pos;
        if(ch == '"'){
            pos++;
            std::string skip_symbols;
            while(*pos == ' ' || *pos == '\n'){
                skip_symbols += *pos;
                pos++;
            }

            if(*pos == ',' || *pos == '}' || *pos == ']' || *pos == ':' || pos == end){
                break;
            } 
            
            line += '\"' + skip_symbols;
            input.putback(*pos);
        }
        else if(ch == '\\'){
            pos++;
            if(pos == end){
                throw ParsingError("string is not closed");
            }
            
            if(*pos == 'n'){
                line += '\n';
            }
            else if(*pos == 'r'){
                line += '\r';
            }
            else if(*pos == 't'){
                line += '\t';
            }
            else if(*pos == '"'){
                line += '\"';
            }
            else if(*pos == '\\'){
                line += '\\';
            }
            else{
                throw ParsingError("string parsing error after \\");
            }
        }
        else{
            line += *pos;
        }
        pos++;
    }

    return Node(move(line));
}

Node LoadDict(istream& input) {
    Dict result;
    bool is_end = false;
    char c;

    for(; input >> c;) {
        if(c == '}'){
            is_end = true;
            break;
        }

        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if(!is_end){
        throw ParsingError("Dict is not closed");
    }

    return Node(move(result));
}

bool IsValidValue(char ch){
    if(ch == EOF || ch == ' ' || ch == ',' || ch == ']' || ch == '}' || ch == '\n' || ch == '\t' || ch == '\r'){
        return true;
    }
    return false;
}

Node LoadBool(istream& input){
    char ch;
    string line;
    input >> ch;
    line += ch;
    size_t size = ch == 't' ? 3 : 4;
    for(size_t i = 0; i < size && input >> ch; i++){
        line += ch;
    }

    ch = static_cast<char>(input.peek());
    if(!IsValidValue(ch)){
        throw ParsingError("Bool parsing error");
    }

    if(line == "true"){
        return Node(true);
    }
    else if(line == "false"){
        return Node(false);
    }
    else {
        throw ParsingError("Bool parsing error");
    }
}

Node LoadNull(istream& input){
    string line = "";
    char ch;
    for(size_t i = 0; i < 4 && input >> ch; i++){
        line += ch;
    }
    if(line == "null" && IsValidValue(ch = static_cast<char>(input.peek()))){
        return Node();
    }
    else {
        throw ParsingError("Error parsing null value");
    }
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if(c == ' '){
        while(true){
            c = static_cast<char>(input.peek());
            if(c != ' '){
                break;
            }
        }
    }

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if(isdigit(c) || c == '-'){
        input.putback(c);
        return LoadNum(input);
    } else if(c == 't' || c == 'f'){
        input.putback(c);
        return LoadBool(input);
    } else if(c == 'n'){
        input.putback(c);
        return LoadNull(input);
    }else{
        throw ParsingError("Error parsing json");
    }
}

}  // namespace

//------------IsTypes-----------

bool Node::IsArray() const{
    if(holds_alternative<Array>(value_)){
        return true;
    }
    return false;
}

bool Node::IsMap() const{
    if(holds_alternative<Dict>(value_)){
        return true;
    }
    return false;
}

bool Node::IsInt() const{
    if(holds_alternative<int>(value_)){
        return true;
    }
    return false;
}

bool Node::IsString() const{
    if(holds_alternative<string>(value_)){
        return true;
    }
    return false;
}

bool Node::IsBool() const{
    if(holds_alternative<bool>(value_)){
        return true;
    }
    return false;
}

bool Node::IsDouble() const{
    if(holds_alternative<double>(value_) || holds_alternative<int>(value_)){
        return true;
    }
    return false;
}

bool Node::IsPureDouble() const{
    if(holds_alternative<double>(value_)){
        return true;
    }
    return false;
}

bool Node::IsNull() const{
    if(holds_alternative<nullptr_t>(value_)){
        return true;
    }
    return false;
}

//------------AsTypes-----------

const Array& Node::AsArray() const {
    if(auto* value = get_if<Array>(&value_)){
        return *value;
    }
    throw std::logic_error("value has type not like array");
}

const Dict& Node::AsMap() const {
    if(auto* value = get_if<Dict>(&value_)){
        return *value;
    }
    throw std::logic_error("value has type not like dict");
}

int Node::AsInt() const {
    if(auto* value = get_if<int>(&value_)){
        return *value;
    }
    throw std::logic_error("value has type not like int");
}

const string& Node::AsString() const {
    if(auto* value = get_if<string>(&value_)){
        return *value;
    }
    throw std::logic_error("value has type not like string");
}

bool Node::AsBool() const{
    if(auto* value = get_if<bool>(&value_)){
        return *value;
    }
    throw std::logic_error("value has type not like bool");
}

double Node::AsDouble() const{
    if(auto* value_double = get_if<double>(&value_)){
        return *value_double;
    }
    else if(auto* value_int = get_if<int>(&value_)){
        return static_cast<double>(*value_int);
    }
    throw std::logic_error("value has type not like double or int");
}

//---------operator---------

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.value_ == rhs.value_;
}

bool operator!=(const Node& lhs, const Node& rhs){
    return lhs.value_ != rhs.value_;
}

bool operator==(const Document& lhs, const Document& rhs){
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs){
    return lhs.GetRoot() != rhs.GetRoot();
}

//-----------Document-----------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

//----------NodePrinter----------

ostream& NodePrinter::operator()(nullptr_t) const{
    out << "null";
    return out;
}
ostream& NodePrinter::operator()(std::string value) const{
    using namespace std;

    out << "\""; 

    auto pos = value.begin();
    while(true){
        if(pos == value.end()){
            break;
        }

        else if(*pos == '\n'){
            out << "\\n";
        }
        else if(*pos == '\r'){
            out << "\\r";
        }
        else if(*pos == '\t'){
            out << "\\t";
        }
        else if(*pos == '\"'){
            out << "\\\"";
        }
        else if(*pos == '\\'){
            out << "\\\\";
        }
        else {
            out << *pos;
        }
        pos++;
    }
    out << "\"";
    return out;
}
ostream& NodePrinter::operator()(int value) const{
    out << value;
    return out;
}
ostream& NodePrinter::operator()(double value) const{
    out << value;
    return out;
}
ostream& NodePrinter::operator()(bool value) const{
    out << boolalpha << value;
    return out;
}
ostream& NodePrinter::operator()(Array array) const{
    bool first = true;
    out << std::string(bracket_indent == 0 ? 0 : (bracket_indent - 1)*4, ' ') << "[\n";
    for(const Node& elem : array){
        if(first == false){
            out << ",";
            out << "\n";
        }
        else{
            first = false;
        }
        out << std::string(indent*4, ' ');
        visit(NodePrinter{bracket_indent, indent + 1, out}, elem.value_);
    }
    out << "\n" << std::string(indent == 0 ? 0 : (indent - 1)*4, ' ') << "]";
    return out;
}
ostream& NodePrinter::operator()(Dict map) const{
    bool first = true;
    out << std::string(bracket_indent*4, ' ') << "{\n";
    for(auto [key, value] : map){
        if(first == false){
            out << ",\n";
        }
        else{
            first = false;
        }
        out << std::string(indent*4, ' ') << " \"" << key << "\"" << ": ";
        visit(NodePrinter{0, indent + 1, out}, value.value_);
    }
    out << "\n" << std::string(indent == 0 ? 0 : (indent - 1)*4, ' ') << "}";
    return out;
}
ostream& NodePrinter::Print(const Node& node) const{
    visit(*this, node.value_);
    return out;
}

//----------Another--------------

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    NodePrinter{0, 1, output}.Print(doc.GetRoot());
}

}  // namespace json