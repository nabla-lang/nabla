#include "syntax_tree.h"

namespace nabla {

[[nodiscard]] auto
to_string(TypeID type_id) -> const char*
{
  switch (type_id) {
    case TypeID::float_:
      return "float";
    case TypeID::int_:
      return "int";
    case TypeID::string:
      return "string";
  }
  return "";
}

} // namespace nabla