#include "json_builder.h"

namespace json {

    namespace details{
        BaseContext::BaseContext(Builder& builder) : builder_(builder){}

        Node BaseContext::Build(){
            if(builder_.GetNodeStack().empty() && !builder_.GetNode().IsNull()){
                return builder_.GetNode();
            }
            throw std::logic_error("invalid json document");
        }

        DictItemContext BaseContext::StartDict(){
            auto& node_stack_ = builder_.GetNodeStack();
            auto& node_ = builder_.GetNode();
            int& count_dict_ = builder_.GetDictCount();

            if(node_stack_.empty() && node_.IsNull()){
                count_dict_++;
                Node* node = new Node(Dict());
                node_stack_.emplace_back(node);
                return DictItemContext(builder_);
            }
            else if(!node_stack_.empty()){
                if(node_stack_.back()->IsArray()){
                    count_dict_++;
                    Node* node = new Node(Dict());
                    node_stack_.emplace_back(node);
                    return DictItemContext(builder_);
                }
                else if(node_stack_.back()->IsDict()){
                    if(!count_dict_){
                        throw std::logic_error("the key for the value is not set");
                    }
                    
                    count_dict_++;
                    Node* node = new Node(Dict());
                    node_stack_.emplace_back(node);
                    return DictItemContext(builder_);
                }
            }
            
            throw std::logic_error("invalid StartDict function call");
        }

        ArrayContex BaseContext::StartArray(){
            auto& node_stack_ = builder_.GetNodeStack();
            auto& node_ = builder_.GetNode();
            int& count_dict_ = builder_.GetDictCount();

            if(node_stack_.empty() && node_.IsNull()){
                Node* array = new Node(Array());
                node_stack_.emplace_back(array);
                return ArrayContex(builder_);
            }
            else if(!node_stack_.empty()){
                if(node_stack_.back()->IsArray()){
                    Node* array = new Node(Array());
                    node_stack_.emplace_back(array);
                    return ArrayContex(builder_);
                }
                else if(node_stack_.back()->IsDict()){
                    if(!count_dict_){
                        throw std::logic_error("the key for the value is not set");
                    }

                    Node* array = new Node(Array());
                    node_stack_.emplace_back(array);
                    return ArrayContex(builder_);
                }
            }
            
            throw std::logic_error("invalid StartArray function call");
        }   

        DictValueContext BaseContext::Key(std::string key){
            auto& node_stack_ = builder_.GetNodeStack();
            int& count_dict_ = builder_.GetDictCount();
            auto& key_ = builder_.GetKeys();

            if(count_dict_){
                if(node_stack_.back()->IsDict()){
                    if(count_dict_ == static_cast<int>(key_.size())){
                        throw std::logic_error("the key has already been set");
                    }

                    key_.push_back(key);
                    return DictValueContext(builder_);
                }
            }

            throw std::logic_error("invalid Key function call");
        }

        BaseContext BaseContext::Value(Node node){
            auto& node_stack_ = builder_.GetNodeStack();
            int& count_dict_ = builder_.GetDictCount();
            auto& key_ = builder_.GetKeys();

            if(!node_stack_.empty()){
                if(node_stack_.back()->IsArray()){
                    auto& array = std::get<Array>(node_stack_.back()->GetValue());
                    array.emplace_back(std::move(node));
                    node_stack_.back()->GetValue() = array;
                    return *this;
                }
                else if(node_stack_.back()->IsDict()){
                    if(!count_dict_ || static_cast<int>(key_.size()) != count_dict_){
                        throw std::logic_error("the key for the value is not set");
                    }

                    auto& dict = std::get<Dict>(node_stack_.back()->GetValue());
                    dict.emplace(key_.back(), std::move(node));
                    key_.pop_back();
                    node_stack_.back()->GetValue() = dict;
                    return *this;
                }
            }
            
            throw std::logic_error("invalid Value function call");
        }

        BaseContext BaseContext::EndDict(){
            auto& node_stack_ = builder_.GetNodeStack();
            auto& node_ = builder_.GetNode();
            int& count_dict_ = builder_.GetDictCount();
            auto& key_ = builder_.GetKeys();

            if(!node_stack_.empty()){
                if(node_stack_.back()->IsDict()){
                    Node node = std::get<Dict>(node_stack_.back()->GetValue());
                    node_stack_.pop_back();
                    if(node_.IsNull() && node_stack_.empty()){
                        node_ = std::move(node);
                        count_dict_--;
                        return BaseContext(builder_);
                    }
                    else if(node_stack_.back()->IsArray()){
                        auto& array = std::get<Array>(node_stack_.back()->GetValue());
                        array.emplace_back(std::move(node));
                        node_stack_.back()->GetValue() = array;
                        count_dict_--;
                        return BaseContext(builder_);
                    }
                    else if(node_stack_.back()->IsDict()){
                        if(!count_dict_){
                            throw std::logic_error("the key for the value is not set");
                        }

                        auto& dict = std::get<Dict>(node_stack_.back()->GetValue());
                        dict.emplace(key_.back(), std::move(node));
                        key_.pop_back();
                        node_stack_.back()->GetValue() = dict;
                        count_dict_--;
                        return BaseContext(builder_);
                    }
                }
            }

            throw std::logic_error("invalid EndDict function call");
        }
        BaseContext BaseContext::EndArray(){
            auto& node_stack_ = builder_.GetNodeStack();
            auto& node_ = builder_.GetNode();
            int& count_dict_ = builder_.GetDictCount();
            auto& key_ = builder_.GetKeys();

            if(!node_stack_.empty()){
                if(node_stack_.back()->IsArray()){
                    Node node = std::get<Array>(node_stack_.back()->GetValue());
                    node_stack_.pop_back();
                    if(node_.IsNull() && node_stack_.empty()){
                        node_ = std::move(node);
                        return BaseContext(builder_);
                    }
                    else if(node_stack_.back()->IsArray()){
                        auto& array = std::get<Array>(node_stack_.back()->GetValue());
                        array.emplace_back(std::move(node));
                        node_stack_.back()->GetValue() = array;
                        return BaseContext(builder_);
                    }
                    else if(node_stack_.back()->IsDict()){
                        if(!count_dict_ || static_cast<int>(key_.size()) != count_dict_){
                            throw std::logic_error("the key for the value is not set");
                        }

                        auto& dict = std::get<Dict>(node_stack_.back()->GetValue());
                        dict.emplace(key_.back(), node);
                        key_.pop_back();
                        node_stack_.back()->GetValue() = dict;
                        return BaseContext(builder_);
                    }
                }
            }

            throw std::logic_error("invalid EndArray function call");
        }

        ////ArrayContext

        ArrayContex::ArrayContex(json::Builder& builder) : BaseContext(builder){}
        ArrayContex ArrayContex::Value(Node node){
            auto& node_stack_ = builder_.GetNodeStack();
            auto& array = std::get<Array>(node_stack_.back()->GetValue());

            array.emplace_back(std::move(node));
            node_stack_.back()->GetValue() = array;
            return ArrayContex(builder_);
        }

        ////DictContext

        DictItemContext::DictItemContext(json::Builder& builder) : BaseContext(builder){}
        DictValueContext::DictValueContext(json::Builder& builder) : BaseContext(builder){}

        DictItemContext DictValueContext::Value(Node node){
            auto& node_stack_ = builder_.GetNodeStack();
            int& count_dict_ = builder_.GetDictCount();
            auto& key_ = builder_.GetKeys();

            if(!count_dict_ || static_cast<int>(key_.size()) != count_dict_){
                throw std::logic_error("the key for the value is not set");
            }

            auto& dict = std::get<Dict>(node_stack_.back()->GetValue());
            dict.emplace(key_.back(), std::move(node));
            key_.pop_back();
            node_stack_.back()->GetValue() = dict;
            return DictItemContext(builder_);
        }

        ////BuildContext

        BuildContext::BuildContext(json::Builder& builder) : BaseContext(builder){}
    }

    details::DictItemContext Builder::StartDict(){
        count_dict_++;
        Node* node = new Node(Dict());
        node_stack_.emplace_back(node);
        return details::DictItemContext(*this);
    }

    details::ArrayContex Builder::StartArray(){
        Node* array = new Node(Array());
        node_stack_.emplace_back(array);
        return details::ArrayContex(*this);
    }

    details::BuildContext Builder::Value(Node node){
        node_ = std::move(node);
        return details::BuildContext(*this);
    }

    Node& Builder::GetNode(){
        return node_;
    }
    std::vector<Node*>& Builder::GetNodeStack(){
        return node_stack_;
    }
    std::vector<std::string>& Builder::GetKeys(){
        return key_;
    }
    int& Builder::GetDictCount(){
        return count_dict_;
    }
}
