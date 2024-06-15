#include "json.h"
#include <optional>
#include <variant>

namespace json {
    
    class Builder;

    namespace details{
        class DictItemContext;
        class ArrayContex;
        class BuildContext;
        class DictValueContext;

        class BaseContext{
        public:
            BaseContext(json::Builder& builder);

            Node Build();
            DictItemContext StartDict();
            ArrayContex StartArray();
            DictValueContext Key(std::string key);
            BaseContext Value(Node node);
            BaseContext EndDict();
            BaseContext EndArray();
        protected:
            json::Builder& builder_;
        };

        class ArrayContex : private BaseContext{
        public:
            using BaseContext::StartArray;
            using BaseContext::StartDict;
            using BaseContext::EndArray;

            ArrayContex(json::Builder& builder);
            ArrayContex Value(Node node);
        };

        class DictItemContext : private BaseContext{
        public:
            using BaseContext::Key;
            using BaseContext::EndDict;

            DictItemContext(json::Builder& builder);
        };

        class DictValueContext : private BaseContext{
        public:
            using BaseContext::StartDict;
            using BaseContext::StartArray;

            DictValueContext(json::Builder& builder);
            DictItemContext Value(Node node);
        };

        class BuildContext : private BaseContext{
        public:
            using BaseContext::Build;

            BuildContext(json::Builder& builder);
        };
    }

class Builder { 
public:
    details::DictItemContext StartDict();
    details::ArrayContex StartArray();
    details::BuildContext Value(Node node_);

    Node& GetNode();
    std::vector<Node*>& GetNodeStack();
    std::vector<std::string>& GetKeys();
    int& GetDictCount();
    
private:
    std::vector<json::Node*> node_stack_;
    Node node_; 
    std::vector<std::string> key_;
    int count_dict_ = 0;
};

}
