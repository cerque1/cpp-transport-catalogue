#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final {
public:
   /* Реализуйте Node, используя std::variant */
    Node() : value_(){}
    template<typename T>
        Node(T value) : value_(std::move(value)) {
    }

    bool IsArray() const;
    bool IsMap() const;
    bool IsInt() const;
    bool IsString() const;
    bool IsBool() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsNull() const;

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;
    bool AsBool() const;
    double AsDouble() const;

    friend bool operator==(const Node& lhs, const Node& rhs);
    friend bool operator!=(const Node& lhs, const Node& rhs);

    friend struct NodePrinter;

private:
    std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict> value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

struct NodePrinter{
    size_t bracket_indent;
    size_t indent;
    std::ostream& out;

    std::ostream& operator()(std::nullptr_t) const;
    std::ostream& operator()(std::string) const;
    std::ostream& operator()(int) const;
    std::ostream& operator()(double) const;
    std::ostream& operator()(bool) const;
    std::ostream& operator()(Array) const;
    std::ostream& operator()(Dict) const;

    std::ostream& Print(const Node& node) const;
};

void Print(const Document& doc, std::ostream& output);
}  // namespace json

