#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "juntos" ? 0 : 1;
}
