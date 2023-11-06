#pragma once

#include <string>
#include <vector>
#include <optional>

#include "json.h"

namespace json {

class Builder final {
public:
    class BaseContext;
    class KeyItemContext;
    class DictItemContext;
    class ArrayItemContext;

    Builder() = default;

    KeyItemContext Key(std::string);

    Builder& Value(Node::Value value);

    DictItemContext StartDict();

    ArrayItemContext StartArray();

    Builder& EndDict();

    Builder& EndArray();

    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    template <typename T>
    void InsertContainer(T item);
};

template <typename T>
void Builder::InsertContainer(T container) {
    using namespace std::string_literals;

    if (root_.IsNull()) {
        root_.GetValue() = container;
        nodes_stack_.emplace_back(&root_);
    } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(container);
        nodes_stack_.emplace_back(&(std::get<Array>(nodes_stack_.back()->GetValue()).back()));
    } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
        *nodes_stack_.back() = container;
        // reference to new map was pushed onto stack when new key was added, so reference mustn't be added
    } else {
        throw std::logic_error("Wrong place to start array of dictionary"s);
    }
}

class Builder::BaseContext {
public:
    BaseContext(Builder& builder) : builder_(builder) {};
    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

private:
    Builder& builder_;
};

class Builder::KeyItemContext : public BaseContext {
public:
    KeyItemContext(Builder& builder) : BaseContext(builder) {};
    KeyItemContext Key(std::string key) = delete;
    DictItemContext Value(Node::Value value);
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

class Builder::DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& builder) : BaseContext(builder) {};
    Builder& Value(Node value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class Builder::ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& builder) : BaseContext(builder) {};
    KeyItemContext Key(std::string key) = delete;
    ArrayItemContext Value(Node::Value value);
    Builder& EndDict() = delete;
};

}