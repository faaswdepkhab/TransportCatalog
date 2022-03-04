#pragma once

#include <map>
#include <vector>
#include <variant>
#include <cstddef>
#include <string>

#include "json.h"

namespace json {

    
using namespace std;    

using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
class Builder;    

class KeyContext;
class DictItemContext;
class ArrayContext;
class ValueDictContext;    

// общий предок    
class CommonContext{
public:    
    CommonContext(Builder &builder_): builder(builder_) {}
protected:
    Builder &builder;
    
    Builder& Value(NodeValue value);
    KeyContext Key(string key);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayContext StartArray();
    Builder& EndArray();
};  
    
    
    
//  ключ
class KeyContext:public CommonContext {
public:    
    KeyContext(Builder &builder): CommonContext(builder) {}
    
    ValueDictContext Value(NodeValue value);
    using CommonContext::StartDict;
    using CommonContext::StartArray;
};
    
    
// словарь    
class DictItemContext:public CommonContext {
public:    
    DictItemContext(Builder &builder): CommonContext(builder) {}
    
    using CommonContext::Key;
    using CommonContext::EndDict;

};
    
// массив    
class ArrayContext:public CommonContext {
public:    
    ArrayContext(Builder &builder_): CommonContext(builder_) {}
    
    ArrayContext Value(NodeValue value);
    using CommonContext::StartDict;
    using CommonContext::StartArray;
    using CommonContext::EndArray;
    

};
    
// значение после ключа
class ValueDictContext:public CommonContext {
public:    
    ValueDictContext(Builder &builder_): CommonContext(builder_) {}

    using CommonContext::Key;
    using CommonContext::EndDict;

};    
    
class Builder {

public:
    Builder& Value(NodeValue value);
    KeyContext Key(string key);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayContext StartArray();
    Builder& EndArray();
    Node Build();
    
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool isEmpty = true;
    bool hasKey = false;
    string keyValue;
    
    void UpdateStack();
};
    
} // namespace json 