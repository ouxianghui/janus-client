
/*
    This library enables normal class to be serialized or deserialized with json object.

    Example 1: serialization for customized structs or classes

    struct B
    {
        int id;
        string name;
        string description;
        JSON_SERIALIZE(JM(id), JM(name), JM(description))
    };

    struct A
    {
        int id;
        vector<B> children;
        JSON_SERIALIZE(JM(id), JM(children))
    };

    void main()
    {
        A a = createObjectA();
        auto json = a.jserialize();

        A a2;
        a2.jdeserialize(json);
    }

    Example 2:  usage of json_serialize or json_deserialize
    void main()
    {
        using namespace json11;

        vector<Child> children = initializeChildren();

        Json::object jobj;
        json_serialize(jobj,
                    "name", "Tom",
                    "age", 15
                    "children", children);

        string name;
        int age;
        json_deserialize(jobj,
                    "name", name,
                    "age", age,
                    "children", children);
    }
*/
#pragma once

#include "absl/types/optional.h"
#include "stringable.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <iostream>
#include <vector>
#include <map>
#include <stdexcept>
#include "string_algo.hpp"

class JsonParsingFailed : public std::runtime_error
{
public:
    JsonParsingFailed(const std::string& raw, int error)
        : std::runtime_error("raw:" + raw + "\r\nwith error:\r\n")
    {}
};

class JsonMissingKey : public std::runtime_error
{
public:
    JsonMissingKey(const rapidjson::Value& json, const std::string& name)
        : std::runtime_error("JsonMissingKey:" + name + "\r\nwith json:\r\n" + string_algo::to_string(json))
    {}
};

class JsonTypeMismatch : public std::runtime_error
{
public:
    JsonTypeMismatch(const rapidjson::Value& json, const std::string& name) : std::runtime_error("JsonTypeMismatch:" + name + "\r\nwith json:\r\n" + string_algo::to_string(json))
    {}
};

using namespace absl;

//This macro declares which members are serializable
#define JSON_SERIALIZE(...)\
    rapidjson::Document jserialize(rapidjson::Document::AllocatorType* allocator = nullptr)const\
                                {\
        rapidjson::Document j(allocator);\
		j.SetObject();\
        json_serialize(j, __VA_ARGS__);\
        return j;\
                                }\
    void jdeserialize(const rapidjson::Value& j)\
                                {\
        json_deserialize(j, __VA_ARGS__);\
                                }

#define JSON_NO_DSERIALIZE(...)\
rapidjson::Document jserialize(rapidjson::Document::AllocatorType* allocator = nullptr)const\
{\
rapidjson::Document j(allocator);\
j.SetObject();\
json_serialize(j, __VA_ARGS__);\
return j;\
}

#define JSON_NO_SERIALIZE()\
rapidjson::Document jserialize(rapidjson::Document::AllocatorType* allocator = nullptr)const {return rapidjson::Document(allocator);}\
void jdeserialize(const rapidjson::Document& j) {}
//using the member name as the json key.
#define JM(v)   #v, v

#define RC_KEY_VALUE(v)   #v, v

struct EmptyJsonType {
    JSON_NO_SERIALIZE()
};

struct JsonContent
{
    absl::optional<std::string> type;
    absl::optional<std::string> content;
    JSON_NO_SERIALIZE()
};

template<typename Model>
struct BatchModel
{
    absl::optional<std::vector<Model>> records;
    JSON_NO_SERIALIZE()
};

struct BatchJsonModel : public BatchModel<JsonContent>
{
};

inline rapidjson::Document stringToJson(const std::string& data) {
	rapidjson::Document d;
	d.Parse(data.c_str());
	if (d.HasParseError()) {
		throw JsonParsingFailed(data, d.GetParseError());
	}
	else
		return d;
}

namespace rapidjson {

//deserialize native values.
struct JsonValueAdapter {
    void operator()(const Value& j, const std::string& name, int64_t& value) {
        if(false == j.IsInt64())
            throw JsonTypeMismatch(j, name);
        value = j.GetInt64();
    }
    void operator()(const Value& j, const std::string& name, int& value) {
        if(false == j.IsInt())
            throw JsonTypeMismatch(j, name);
        value = (int)j.GetInt();
    }
    void operator()(const Value& j, const std::string& name, double& value) {
        if(false == j.IsDouble())
            throw JsonTypeMismatch(j, name);
        value = j.GetDouble();
    }
    void operator()(const Value& j, const std::string& name, float& value) {
        if(false == j.IsDouble())
            throw JsonTypeMismatch(j, name);
        value = (float)j.GetDouble();
    }
    void operator()(const Value& j, const std::string& name, std::string& value) {
        if(false == j.IsString())
            throw JsonTypeMismatch(j, name);
        value = j.GetString();
    }
    void operator()(const Value& j, const std::string& name, bool& value) {
        if(false == j.IsBool())
            throw JsonTypeMismatch(j, name);
        value = j.GetBool();
    }
    //for container
    template<typename Type>
    void operator()(const Value& j, const std::string& name, std::vector<Type>& value) {
        if(false == j.IsArray())
            throw JsonTypeMismatch(j, name);
        value.clear();
		for (auto item = j.Begin(); j.End() != item; ++item) {
            Type v;
            JsonValueAdapter()(*item, name, v);
            value.push_back(v);
        }
    }
    
    //for container
    template<typename Type>
    void operator()(const Value& j, const std::string& name, std::map<std::string, Type>& value) {
        if(false == j.IsObject())
            throw JsonTypeMismatch(j, name);
        value.clear();
        for (auto item = j.MemberBegin(); j.MemberEnd() != item; ++item) {
            std::string k{item->name.GetString(), item->name.GetStringLength()};
            Type v;
            JsonValueAdapter()(item->value, k, v);
            value[k] = v;
        }
    }

	void set(Value& j, int64_t value, Document::AllocatorType& alloc) {
		j.SetInt64(value);
	}
	void set(Value& j, int value, Document::AllocatorType& alloc) {
		j.SetInt(value);
	}
	void set(Value& j, double value, Document::AllocatorType& alloc) {
		j.SetDouble(value);
	}
	void set(Value& j, const std::string& value, Document::AllocatorType& alloc) {
		j.SetString(value.c_str(), (SizeType)value.size());
	}
	void set(Value& j, const char* value, Document::AllocatorType& alloc) {
		j.SetString(value, alloc);
	}
	void set(Value& j, bool value, Document::AllocatorType& alloc) {
		j.SetBool(value);
	}
	template<typename Type>
	void set(Value& j, const std::vector<Type>& value, Document::AllocatorType& alloc) {
		j.SetArray();
		for (auto& v : value) {
			Value jv;
			JsonValueAdapter().set(jv, v, alloc);
			j.PushBack(jv, alloc);
		}
	}
    
    template<typename Type>
    void set(Value& j, const std::map<std::string, Type>& value, Document::AllocatorType& alloc) {
        j.SetObject();
        for (auto& v : value) {
            Value jk, jv;
            jk.SetString(v.first.c_str(), (SizeType)v.first.size());
            JsonValueAdapter().set(jv, v.second, alloc);
            j.AddMember(jk, jv, alloc);
        }
    }
};

//the functor for serializing native types, such as int, bool, string, enum, vector<native_type>, etc.
template<typename Key, typename Type, bool IsEnum = std::is_enum<Type>::value>
struct JsonSerializerPod {
    void forward(Document& jlist, const Key& name, const Type& value) {
        jlist[name] = (int64_t)value;
    }
    void backward(const Value& jlist, const Key& name, Type& value) {
		auto member = jlist.FindMember(name);
		if (jlist.MemberEnd() == member)
            throw JsonMissingKey(jlist, name);
        value = (Type)member->value.GetInt64();
    }
};

inline Value JsonKey(const std::string& key, Document::AllocatorType& alloc) {
	return Value(StringRef(key.c_str(), key.size()), alloc);
}
/*
inline std::string StringKey(const std::string& key) {
    return key;
}

inline std::string StringKey(const GenericStringRef<char>& key) {
    return key.s;
}*/
    
template<typename Key, typename Type>
struct JsonSerializerPod<Key, Type, false> {
    void forward(Document& jlist, const Key& name, const Type& value) {
		Value jv;
		JsonValueAdapter().set(jv, value, jlist.GetAllocator());
		jlist.AddMember(JsonKey(name, jlist.GetAllocator()), jv, jlist.GetAllocator());
    }

    void backward(const Value& jlist, const Key& name, Type& value) {
		auto member = jlist.FindMember(name);
		if(jlist.MemberEnd() == member)
            throw JsonMissingKey(jlist, name);
        JsonValueAdapter()(member->value, name, value);
    }
};

//serializer for customized types
template<typename Key, typename Type>
struct JsonSerializer {
    void forward(Document& jlist, const Key& name, const Type& value) {
		jlist.AddMember(name, value.jserialize(&jlist.GetAllocator()), jlist.GetAllocator());
    }
    void backward(const Value& jlist, const Key& name, Type& value) {
        if (!jlist.IsObject()) {
            throw JsonTypeMismatch(jlist, name);
        }
		auto member = jlist.FindMember(name);
        if (jlist.MemberEnd() == member)
            throw JsonMissingKey(jlist, name);
        value.jdeserialize(member->value);
    }
};

template<typename Key, typename Type>
struct JsonSerializer < Key, optional<Type> > {
    void forward(Document& jlist, const Key& name, const optional<Type>& value) {
        if (value)
            json_serialize(jlist, name, *value);
    }
    void backward(const Value& jlist, const Key& name, optional<Type>& value) {
        if (!jlist.IsObject()) {
            throw JsonTypeMismatch(jlist, name);
        }
        if (jlist.HasMember(name)) {
            auto memberV = jlist.FindMember(name);
            if (!memberV->value.IsNull())
            {
                Type v;
                json_deserialize(jlist, name, v);
                value = v;
            }
        }
    }
};
//serializer for vector of customized types
template<typename Key, typename Type>
struct JsonSerializerVector {
    void forward(Document& jlist, const Key& name, const Type& value) {
		Document arr(&jlist.GetAllocator());
		arr.SetArray();
        for (const auto& item : value) {
            // arr.PushBack(item.jserialize(&jlist.GetAllocator()), jlist.GetAllocator());
            auto itemDoc = item.jserialize(&jlist.GetAllocator());
            arr.PushBack(Value().Swap(itemDoc).Move(), jlist.GetAllocator());
        }
		jlist.AddMember(name, arr, jlist.GetAllocator());
    }
    void backward(const Value& jlist, const Key& name, Type& value) {
        value.clear();
        auto it = jlist.FindMember(name);
        if (jlist.MemberEnd() == it)
            return; //not throwing JsonMissingKey for vectors
//            throw JsonMissingKey(jlist, name);
        if(false == it->value.IsArray())
            throw JsonTypeMismatch(it->value, name);
		for (auto jitem = it->value.Begin(); it->value.End() != jitem; ++jitem) {
            typename Type::value_type v;
            v.jdeserialize(*jitem);
            value.push_back(v);
        }
    }
};

    
template<typename Key, typename Type>
struct JsonSerializerMap {
    void forward(Document& jlist, const Key& name, const Type& value) {
        Document obj(&jlist.GetAllocator());
        obj.SetObject();
        for (const auto& pair : value) {
            Value name;
            JsonValueAdapter().set(name, pair.first, jlist.GetAllocator());
            // obj.AddMember(name, pair.second.jserialize(&jlist.GetAllocator()), jlist.GetAllocator());
            auto itemDoc = pair.second.jserialize(&jlist.GetAllocator());
            obj.AddMember(name.Move()
                          , Value().Swap(itemDoc).Move()
                          , jlist.GetAllocator());
        }
        jlist.AddMember(name, obj, jlist.GetAllocator());
    }

    void backward(const Value& jlist, const Key& name, Type& value) {
        value.clear();
        auto it = jlist.FindMember(name);
        if (jlist.MemberEnd() == it)
            return;

        if(false == it->value.IsObject())
            throw JsonTypeMismatch(it->value, name);

        for (auto jitem = it->value.MemberBegin(); it->value.MemberEnd() != jitem; ++jitem) {
            if (jitem->name.IsObject()) {
                throw JsonTypeMismatch(jitem->name, std::string(name) + "/*");
            }
            
            using PairType = typename Type::value_type;
            using KeyType = typename Type::key_type;
            using MappedType = typename Type::mapped_type;
            
            KeyType const key { jitem->name.GetString(), jitem->name.GetStringLength() };
            MappedType mapped;
            mapped.jdeserialize(jitem->value);
            value[key] = mapped;
        }
    }
};

//determines that if a type is a supported container. currently only vector is supported.
template<typename T>
struct is_supported_container {
    static const bool value = false;
};
template<typename T>
struct is_supported_container < std::vector<T> > {
    static const bool value = true;
};
template <class T>
struct is_supported_container< std::map<std::string, T> >  {
    static const bool value = true;
};
    
template<typename T>
struct is_vector {
    static const bool value = false;
};
template<typename T>
struct is_vector < std::vector<T> > {
    static const bool value = true;
};


//determines if it is a vector of supported types, such as vector<int>, vector<string>
template<typename Type>
struct is_vector_of_native {
    static const bool value = false;
};
template<typename Type>
struct is_map_of_native {
    static const bool value = false;
};
template<typename Type>
struct is_native_type;
template<typename Type>
struct is_vector_of_native < std::vector<Type> > {
    static const bool value = is_native_type<Type>::value;
};
template<typename Type>
struct is_map_of_native < std::map<std::string, Type> > {
    static const bool value = is_native_type<Type>::value;
};

//determines that if a type is supported type
template<typename Type>
struct is_native_type {
    static const bool value =
        std::is_integral<Type>::value ||
        std::is_enum<Type>::value ||
        std::is_floating_point<Type>::value ||
        //   std::is_same<Type, std::string>::value ||
        std::is_convertible<Type, std::string>::value ||
        is_vector_of_native<Type>::value ||
        is_map_of_native<Type>::value;
};

//selectors of types,
//native type is dipatched to JsonSerializerPod
//vector of non-native is dispatched to JsonSerializerVector
//single non-native type is dispatched to JsonSerializer
template<typename Key, typename Type, typename... Tail>
inline void json_serialize(Document& jlist, const Key& name, const Type& value, const Tail& ... tail) {
    using namespace std;
    typename conditional < is_native_type<Type>::value,
        JsonSerializerPod<Key, Type>,
        typename conditional<!is_supported_container<Type>::value,
            JsonSerializer<Key, Type>,
            typename conditional<is_vector<Type>::value,
                JsonSerializerVector<Key, Type>,
                JsonSerializerMap<Key, Type>
            >::type
        >::type
    >::type().forward(jlist, name, value);
    json_serialize(jlist, tail...);
}
    
//terminus
inline void json_serialize(Document& jlist) {
}

//the same basis as json_serialize
template<typename Key, typename Type, typename... Tail>
inline void json_deserialize(const Value& jlist, const Key& name, Type& value, Tail& ... tail) {
    using namespace std;
    typename conditional < is_native_type<Type>::value,
        JsonSerializerPod<Key, Type>,
        typename conditional<!is_supported_container<Type>::value,
            JsonSerializer<Key, Type>,
            typename conditional<is_vector<Type>::value,
                JsonSerializerVector<Key, Type>,
                JsonSerializerMap<Key, Type>
            >::type
        >::type
    >::type().backward(jlist, name, value);
    json_deserialize(jlist, tail...);
}

//terminus
inline void json_deserialize(const Value& jlist) {
}
}

template<>
struct stringable<rapidjson::Value>
{
	std::string operator()(const rapidjson::Value& json) {
		using namespace rapidjson;
		StringBuffer sb;
		PrettyWriter<StringBuffer> writer(sb);
		json.Accept(writer);
		return sb.GetString();
	}
	rapidjson::Value convert_back(const std::string& s) {
		throw std::runtime_error("unable to convert string to json value");
	}
};

template<>
struct stringable<rapidjson::Document>
{
	std::string operator()(const rapidjson::Document& json) {
		return stringable<rapidjson::Value>()(json);
	}
	rapidjson::Document convert_back(const std::string& s) {
		using namespace rapidjson;
		Document json;
		json.Parse(s.c_str());
		return json;
	}
};

template<typename... Params>
inline rapidjson::Document toJson(const Params&... params) {
	rapidjson::Document doc;
    doc.SetObject();
	json_serialize(doc, params...);
	return doc;
}

template<typename Type>
inline rapidjson::Document toJson(const Type& value) {
    return value.jserialize();
}

template<typename... Params>
inline std::string toJsonString(const Params&... params) {
    return string_algo::to_string(toJson(params...));
}

template<>
inline std::string toJsonString<rapidjson::Document>(const rapidjson::Document& json) {
    return string_algo::to_string(json);
}

template<typename Type>
inline std::shared_ptr<Type> fromJsonString(const std::string& data, bool bCheckValidObject = false) {
    std::shared_ptr<Type> object = std::make_shared<Type>();
    rapidjson::Document json = stringToJson(data);
    // add proected code to avoid crash
    // if server error is not a valid object, for example is a array;
    if (bCheckValidObject && !json.IsObject()) {
        return nullptr;
    }
    object->jdeserialize(json);
    return object;
}

template<typename Type>
inline std::shared_ptr<Type> fromJsonString(const std::string& data, std::string& error) {
    std::shared_ptr<Type> object = std::make_shared<Type>();
    try {
        rapidjson::Document json = stringToJson(data);
        object->jdeserialize(json);
    }
    catch (const JsonMissingKey& e) {
        error = e.what();
    }
    catch (const JsonTypeMismatch& e) {
        error = e.what();
    }
    catch (const JsonParsingFailed& e) {
        error = e.what();
    }
    
    return object;
}





