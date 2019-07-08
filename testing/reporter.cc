#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "testing/reporter.h"
#include "third_party/absl/strings/str_cat.h"
#include "third_party/absl/strings/str_join.h"

namespace firebase {
namespace testing {
namespace cppsdk {
ReportRow::ReportRow()
    : fake_(""),
      result_(""),
      platform_(kAny),
      args_(std::vector<std::string>()) {}

ReportRow::ReportRow(std::string fake, std::string result,
                     std::vector<std::string> args)
    : fake_(std::move(fake)),
      result_(std::move(result)),
      platform_(kAny),
      args_(std::move(args)) {}

ReportRow::ReportRow(std::string fake, std::string result, Platform platform,
                     std::initializer_list<std::string> args)
    : fake_(std::move(fake)),
      result_(std::move(result)),
      platform_(platform),
      args_(args) {}

std::string ReportRow::getFake() const { return fake_; }

std::string ReportRow::getResult() const { return result_; }

Platform ReportRow::getPlatform() const { return platform_; }

std::string ReportRow::getPlatformString() const {
  switch (platform_) {
    case kAny:
      return "any";
    case kAndroid:
      return "android";
    case kIos:
      return "ios";
    default:
      return "undefined";
  }
}

std::vector<std::string> ReportRow::getArgs() const { return args_; }

bool ReportRow::operator==(const ReportRow& other) const {
  if (fake_ != other.fake_ || result_ != other.result_ ||
      args_.size() != other.args_.size()) {
    return false;
  }
  for (int i = 0; i < args_.size(); i++) {
    if (args_[i] != other.args_[i]) return false;
  }
  return true;
}

bool ReportRow::operator!=(const ReportRow& other) const {
  return !((*this) == other);
}

bool ReportRow::operator<(const ReportRow& other) const {
  return fake_ < other.fake_;
}

std::string ReportRow::toString() const {
  return absl::StrCat(fake_, " ", result_, " ", getPlatformString(), " [",
                      absl::StrJoin(args_, " "), "]");
}

::std::ostream& operator<<(::std::ostream& os, const ReportRow& r) {
  return os << r.toString();
}

void Reporter::addExpectation(const ReportRow& expectation) {
  if (expectation.getPlatform() == kAny) {
    expectations_.push_back(expectation);
    return;
  }
#if defined(FIREBASE_ANDROID_FOR_DESKTOP) || defined(__ANDROID__)
  if (expectation.getPlatform() == kAndroid)
    expectations_.push_back(expectation);
#elif defined(__APPLE__)
  if (expectation.getPlatform() == kIos) expectations_.push_back(expectation);
#endif  // defined(FIREBASE_ANDROID_FOR_DESKTOP) || defined(__ANDROID__)
}

void Reporter::addExpectation(std::string fake, std::string result,
                              Platform platform,
                              std::initializer_list<std::string> args) {
  addExpectation(ReportRow(std::move(fake), std::move(result), platform, args));
}

std::vector<ReportRow> Reporter::getExpectations() {
  std::sort(expectations_.begin(), expectations_.end());
  return expectations_;
}

std::vector<ReportRow> Reporter::getFakeReports() {
  std::vector<std::string> fake_fucntions = getAllFakes();
  std::vector<ReportRow> fake_reports;

  for (const auto& fake_function : fake_fucntions) {
    ReportRow r = ReportRow(fake_function, getFakeResult(fake_function),
                            getFakeArgs(fake_function));
    fake_reports.push_back(r);
  }
  std::sort(fake_reports.begin(), fake_reports.end());
  return fake_reports;
}

}  // namespace cppsdk
}  // namespace testing
}  // namespace firebase