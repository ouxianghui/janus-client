#pragma once

#include <string>
#include <memory>
#include "absl/types/optional.h"
#include "serialization_json.hpp"

#define FIELDS_MAP(...)   \
JSON_SERIALIZE(__VA_ARGS__) \
MODEL_2_STRING()  \
STRING_2_MODEL()


#define FIELDS_MAP_NO_DSERIALIZE(...)   \
JSON_NO_DSERIALIZE(__VA_ARGS__) \
MODEL_2_STRING()  \
STRING_2_MODEL()

#define MODEL_2_STRING() virtual std::string toJsonStr() { return toJsonString(*this); }
#define STRING_2_MODEL() virtual rapidjson::Document toJsonOject() { return toJson(*this); }

namespace vi {
    class Jsonable {
    public:
        virtual ~Jsonable() {};
    };
}
/* jsonable_hpp */
