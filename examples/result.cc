#include "stx/result.h"

// Do take note of the use of namespaces and aliases to make the code more
// readable

namespace fs {
using stx::Ok, stx::Err;

enum class Error { InvalidPath };

template <typename T>
using Result = stx::Result<T, Error>;

// this is just a mock
struct File {
  explicit File(std::string_view) {}
};

auto open(std::string_view path) -> Result<File> {
  if (path.empty()) return Err(Error::InvalidPath);

  File file{path};

  return Ok(std::move(file));
}

}  // namespace fs

int main() {
  fs::Result<fs::File> file = fs::open("...");

  fs::File log_file = fs::open({}).expect("unable to open file");  // panics
}