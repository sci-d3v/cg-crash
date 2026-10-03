#ifndef DETAILED_EXCEPTION_H
#define DETAILED_EXCEPTION_H

#include <sstream>
#include <stdexcept>
#include <string>

// Computer Graphics Namespace (cg)
namespace cg {

class DetailedException : public std::runtime_error {
 private:
  static std::string buildMessage(const std::string& msg,
                                  const std::string& file, int line,
                                  const std::string& func,
                                  const std::string& context) {
    // Keep only the filename
    std::string filename = file;
    auto pos = filename.find_last_of("/\\");
    if (pos != std::string::npos) {
      filename = filename.substr(pos + 1);
    }

    std::ostringstream oss;
    oss << "[ERROR] " << filename << ":" << line << " in " << func
        << "(): " << msg;
    if (!context.empty()) {
      oss << " (" << context << ")";
    }
    return oss.str();
  }

 public:
  DetailedException(const std::string& message, const std::string& context,
                    const char* file, int line, const char* func)
      : std::runtime_error(buildMessage(message, file, line, func, context)) {}
};

}  // namespace cg

// Macro for convenience:
#define THROW_DETAILED(msg, ctx) \
  throw cg::DetailedException((msg), (ctx), __FILE__, __LINE__, __func__)

#endif  // DETAILED_EXCEPTION_H
