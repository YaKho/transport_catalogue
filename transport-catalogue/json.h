#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final 
    : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;
    bool AsBool() const;
    double AsDouble() const;

    bool IsNull() const noexcept;
    bool IsBool() const noexcept;
    bool IsInt() const noexcept;
    bool IsDouble() const noexcept;
    bool IsPureDouble() const noexcept;
    bool IsString() const noexcept;
    bool IsArray() const noexcept;
    bool IsMap() const noexcept;

    bool operator==(const Node& other) const {
        return *this == other.GetValue();
    }
    bool operator!=(const Node& other) const {
        return !(*this == other);
    }
    const Value& GetValue() const;
    Value& GetValue();
};

class Document final {
public:
    Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const {
        return root_ == other.GetRoot();
    }
    bool operator!=(const Document& other) const {
        return !(*this == other);
    }
private:
    Node root_;
};

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    PrintContext(std::ostream& out, int indent_step, int indent)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {}

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json