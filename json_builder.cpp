#include "json_builder.h"

namespace json {

Builder& CommonContext::Value(NodeValue value) {
    return builder.Value(value);
}
    
KeyContext CommonContext::Key(string key) {
    return builder.Key(key);
}
    
DictItemContext CommonContext::StartDict() {
    return builder.StartDict();
}
    
Builder& CommonContext::EndDict() {
    return builder.EndDict();
}
    
ArrayContext CommonContext::StartArray() {
    return builder.StartArray();
}
    
Builder& CommonContext::EndArray() {
    return builder.EndArray();
}
   
ArrayContext ArrayContext::Value(NodeValue value) {
    return ArrayContext(builder.Value(value));
}

ValueDictContext KeyContext::Value(NodeValue value) {
    return ValueDictContext(builder.Value(value));
}  
    
    
Builder &Builder::Value(NodeValue value) {

    Node newNode = std::visit([](auto val){
        return Node(val);
    }, value);

    // Самое начало работы
    if (isEmpty) {
        root_ = newNode;
        isEmpty = false;
        return *this;
    }

    // Если мы внутри словаря и ключ уже добавлен
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && hasKey) {
        const_cast<Dict&>(nodes_stack_.back()->AsDict()).insert({keyValue, newNode});
        hasKey = false;
        return *this;
    }

    // Если мы внутри массива
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(newNode);
        return *this;
    }

    // в остальных случаях выбрасываем исключение
    throw std::logic_error("Incorrect place for value"s);
}
    
KeyContext Builder::Key(string key) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Key use only in dict"s);
    }
    
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key use only in dict"s);
    }
    
    if (hasKey) {
        throw std::logic_error("Multiply key"s);
    }
    
    keyValue = key;
    hasKey = true;
    return KeyContext(*this);
}    

DictItemContext Builder::StartDict() {
    Value(Dict{});
    UpdateStack();
    return DictItemContext(*this);
}
    
Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !(nodes_stack_.back()->IsDict()) || hasKey ) {
        throw std::logic_error("Invalid dict"s);
    }
    nodes_stack_.pop_back();
    return *this;
}    

ArrayContext Builder::StartArray() {
    Value(Array{});
    UpdateStack();
    return ArrayContext(*this);
}    

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !(nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Invalid dict"s);
    }
    nodes_stack_.pop_back();
    return *this;
}    

Node Builder::Build() {
    if (isEmpty) {
        throw std::logic_error("Document is empty"s);
    }
    if (nodes_stack_.size()>0) {
        throw std::logic_error("Document is invalid"s);
    }
    return root_;
}    
    
void Builder::UpdateStack() {
    if (nodes_stack_.empty()) {
        nodes_stack_.push_back(&root_);
        return;
    }
    
    if (nodes_stack_.back()->IsArray()) {
        auto p = &nodes_stack_.back()->AsArray().back();            
        nodes_stack_.push_back(const_cast<Node*>(p));
        return;
    }
    
    if (nodes_stack_.back()->IsDict()) {
        auto p = &nodes_stack_.back()->AsDict().at(keyValue);
        nodes_stack_.push_back(const_cast<Node*>(p));
        return;
    }
}
    
} // namespace json
    