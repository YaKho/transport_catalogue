#include "json.h"

using namespace std;

namespace json {

namespace {

using Number = std::variant<int, double>;

Number ParseNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }
    bool is_int = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return std::stoi(parsed_num);
            } catch (...) {}
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

std::string ParseString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }
    return s;
}

Node LoadString(istream& input) {
    string line = ParseString(input);
    return Node(move(line));
}

Node LoadNull(istream& input) {
    std::string res;
    char c;

    for (int i = 0; i < 4; ++i) {
        if (input.get(c)) {
            res += c;
        }
    }
    if (res != "null") {
        throw ParsingError("Failed to parse null node");
    }
    return Node();
}

Node LoadBool(istream& input) {
    std::string res;
    char c;

    c = static_cast<char>(input.peek());
    int length = (c == 't') ? 4 : 5;

    for (int i = 0; i < length; ++i) {
        if (input.get(c)) {
            res += c;
        }
    }
    if (res != "true"s && res != "false"s) {
        throw ParsingError("Failed to parse bool node");
    }
    if (res == "true") {
        return Node(true);
    } else {
        return Node(false);
    }
}

Node LoadNum(istream& input) {
    auto num = ParseNumber(input);
    if (std::holds_alternative<double>(num)) {
        return Node(std::get<double>(num));
    } else {
        return Node(std::get<int>(num));
    }
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array res;
    char c;
    while (input >> c) {
        if (c == ']') {
            break;
        }
        if (c != ',') {
            input.putback(c);
        }
        res.push_back(LoadNode(input));
    }
    if (c != ']') {
        throw ParsingError("Failed to parse array node");
    }
    return Node(move(res));
}

Node LoadDict(istream& input) {
    Dict res;
    char c;
    input >> c;

    if (c == '}') {
        return Node(Dict{});
    } else {
        input.putback(c);
    }
    while (input >> c) {
        input.putback(c);
        string key;
        auto first_node = LoadNode(input);
        if (first_node.IsString()) {
            key = first_node.AsString();
        } else {
            throw ParsingError("Failed to parse dict key");
        }

        input >> c;
        if (c != ':') {
            throw ParsingError("Failed to parse dict node");
        }
        res.insert({ move(key), LoadNode(input) });

        input >> c;
        if (c == '}') {
            break;
        } else if (c != ',') {
            throw ParsingError("Failed to parse dict");
        }
    }
    if (c != '}') {
        throw ParsingError("Failed to parse dict node");
    }
    return Node(move(res));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (std::isdigit(c) || c == '-') {
        input.putback(c);
        return LoadNum(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        throw ParsingError("Failed to parse document"s);
    }
}

}  // namespace

//------------- Node -------------------------

bool Node::IsNull() const noexcept {
    return std::holds_alternative<std::nullptr_t>(*this);
}
bool Node::IsBool() const noexcept {
    return std::holds_alternative<bool>(*this);
}
bool Node::IsInt() const noexcept {
    return std::holds_alternative<int>(*this);
}
bool Node::IsDouble() const noexcept {
    return std::holds_alternative<double>(*this) ||
        std::holds_alternative<int>(*this);
}
bool Node::IsPureDouble() const noexcept {
    return std::holds_alternative<double>(*this);
}
bool Node::IsString() const noexcept {
    return std::holds_alternative<std::string>(*this);
}
bool Node::IsArray() const noexcept {
    return std::holds_alternative<Array>(*this);
}
bool Node::IsMap() const noexcept {
    return std::holds_alternative<Dict>(*this);
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return std::get<Array>(*this);
    } else {
        throw std::logic_error("Node is not array"s);
    }
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return std::get<Dict>(*this);
    } else {
        throw std::logic_error("Node is not map"s);
    }
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(*this);
    } else {
        throw std::logic_error("Node is not int"s);
    }
}

const string& Node::AsString() const {
    if (IsString()) {
        return std::get<std::string>(*this);
    } else {
        throw std::logic_error("Node is not string"s);
    }
}

bool Node::AsBool() const {
    if (IsBool()) {
        return std::get<bool>(*this);
    } else {
        throw std::logic_error("Node is not bool"s);
    }
}

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(*this);
    } else if (IsInt()) {
        return static_cast<double>(std::get<int>(*this));
    } else {
        throw std::logic_error("Node is not double"s);
    }
}

const Node::Value& Node::GetValue() const {
    return *this;
}

Node::Value& Node::GetValue() {
    return *this;
}

//----------------------- Document -----------------------------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

//--------------------- Print ------------------------------
std::string AddEscapes(const std::string& s) {
    std::string res;
    for (auto c : s) {
        switch (c) {
        case '\"':
            res += "\\\"";
            break;
        case '\r':
            res += "\\r";
            break;
        case '\n':
            res += "\\n";
            break;
        case '\\':
            res += "\\\\";
            break;
        default:
            res += c;
            break;
        }
    }
    return res;
}

void PrintNode(const Node& node, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}
void PrintValue(bool value, const PrintContext& ctx) {
    if (value) {
        ctx.out << "true"sv;
    } else {
        ctx.out << "false"sv;
    }
}
void PrintValue(const Array& value, const PrintContext& ctx) {
    ctx.out << "["sv << std::endl;
    auto val_ctx = ctx.Indented();
    bool first = true;
    for (const auto& i : value) {
        if (!first) {
            ctx.out << ","sv << std::endl;
        } else {
            first = false;
        }
        val_ctx.PrintIndent();
        PrintNode(i, val_ctx);
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "]"sv;
}
void PrintValue(const Dict& value, const PrintContext& ctx) {
    ctx.out << "{"sv << std::endl;
    auto key_ctx = ctx.Indented();
    bool first = true;
    for (const auto& [key, val] : value) {
        if (!first) {
            ctx.out << ","sv << std::endl;
        } else {
            first = false;
        }
        key_ctx.PrintIndent();
        PrintNode({ key }, key_ctx);
        ctx.out << ": "sv;
        PrintNode(val, key_ctx);
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "}"sv;
}
void PrintValue(const std::string value, const PrintContext& ctx) {
    ctx.out << "\""sv << AddEscapes(value) << "\""sv;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit([&ctx](const auto& value) { PrintValue(value, ctx); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext ctx(output, 4, 0);
    PrintNode(doc.GetRoot(), ctx);
}

}  // namespace json