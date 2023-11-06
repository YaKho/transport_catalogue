#include <map>

#include "json_builder.h"

using namespace std::string_literals;

namespace json {

Builder::KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Wrong place for the key : "s + key);
    }
    nodes_stack_.emplace_back(&(std::get<Dict>(nodes_stack_.back()->GetValue())[key]));

    return *this;
}

Builder& Builder::Value(Node::Value value) {
    auto node = std::visit([](Node val) {
        return Node(val);
        }, value);

    if (root_.IsNull()) {
        root_ = node;
    } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(node);
    } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
        *nodes_stack_.back() = node;
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("Wrong place for value : "s + node.AsString());
    }

    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    InsertContainer(Dict{});

    return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
    InsertContainer(Array{});

    return *this;
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Wrong place for end of dictionary"s);
    }
    nodes_stack_.pop_back();

    return *this;
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Wrong place for end of array"s);
    }
    nodes_stack_.pop_back();

    return *this;
}

Node Builder::Build() {
    if (nodes_stack_.empty() && !root_.IsNull()) {
        return root_;
    }
    throw std::logic_error("Wrong place for build"s);
}

Builder::KeyItemContext Builder::BaseContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& Builder::BaseContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}

Builder& Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value value) {
    return BaseContext::Value(std::move(value));
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    return BaseContext::Value(std::move(value));
}

}