/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ART_PROFMAN_PROFILE_ASSISTANT_H_
#define ART_PROFMAN_PROFILE_ASSISTANT_H_

#include <string>
#include <vector>

#include "base/scoped_flock.h"
#include "profile/profile_compilation_info.h"

namespace art {

class ProfileAssistant {
 public:
  // These also serve as return codes of profman and are processed by installd
  // (frameworks/native/cmds/installd/commands.cpp)
  enum ProcessingResult {
    kSuccess = 0,  // Generic success code for non-analysis runs.
    kCompile = 1,
    kSkipCompilation = 2,
    kErrorBadProfiles = 3,
    kErrorIO = 4,
    kErrorCannotLock = 5,
    kErrorDifferentVersions = 6,
  };

  class Options {
   public:
    static constexpr bool kForceMergeDefault = false;
    static constexpr bool kBootImageMergeDefault = false;
    static constexpr uint32_t kMinNewMethodsPercentChangeForCompilation = 20;
    static constexpr uint32_t kMinNewClassesPercentChangeForCompilation = 20;

    Options()
        : force_merge_(kForceMergeDefault),
          boot_image_merge_(kBootImageMergeDefault),
          min_new_methods_percent_change_for_compilation_(
              kMinNewMethodsPercentChangeForCompilation),
          min_new_classes_percent_change_for_compilation_(
              kMinNewClassesPercentChangeForCompilation) {
    }

    bool IsForceMerge() const { return force_merge_; }
    bool IsBootImageMerge() const { return boot_image_merge_; }
    uint32_t GetMinNewMethodsPercentChangeForCompilation() const {
        return min_new_methods_percent_change_for_compilation_;
    }
    uint32_t GetMinNewClassesPercentChangeForCompilation() const {
        return min_new_classes_percent_change_for_compilation_;
    }

    void SetForceMerge(bool value) { force_merge_ = value; }
    void SetBootImageMerge(bool value) { boot_image_merge_ = value; }
    void SetMinNewMethodsPercentChangeForCompilation(uint32_t value) {
      min_new_methods_percent_change_for_compilation_ = value;
    }
    void SetMinNewClassesPercentChangeForCompilation(uint32_t value) {
      min_new_classes_percent_change_for_compilation_ = value;
    }

   private:
    // If true, performs a forced merge, without analyzing if there is a
    // significant difference between the current profile and the reference profile.
    // See ProfileAssistant#ProcessProfile.
    bool force_merge_;
    // Signals that the merge is for boot image profiles. It will ignore differences
    // in profile versions (instead of aborting).
    bool boot_image_merge_;
    uint32_t min_new_methods_percent_change_for_compilation_;
    uint32_t min_new_classes_percent_change_for_compilation_;
  };

  // Process the profile information present in the given files. Returns one of
  // ProcessingResult values depending on profile information and whether or not
  // the analysis ended up successfully (i.e. no errors during reading,
  // merging or writing of profile files).
  //
  // When the returned value is kCompile there is a significant difference
  // between profile_files and reference_profile_files. In this case
  // reference_profile will be updated with the profiling info obtain after
  // merging all profiles.
  //
  // When the returned value is kSkipCompilation, the difference between the
  // merge of the current profiles and the reference one is insignificant. In
  // this case no file will be updated.
  //
  static ProcessingResult ProcessProfiles(
      const std::vector<std::string>& profile_files,
      const std::string& reference_profile_file,
      const ProfileCompilationInfo::ProfileLoadFilterFn& filter_fn
          = ProfileCompilationInfo::ProfileFilterFnAcceptAll,
      const Options& options = Options());

  static ProcessingResult ProcessProfiles(
      const std::vector<int>& profile_files_fd_,
      int reference_profile_file_fd,
      const ProfileCompilationInfo::ProfileLoadFilterFn& filter_fn
          = ProfileCompilationInfo::ProfileFilterFnAcceptAll,
      const Options& options = Options());

 private:
  static ProcessingResult ProcessProfilesInternal(
      const std::vector<ScopedFlock>& profile_files,
      const ScopedFlock& reference_profile_file,
      const ProfileCompilationInfo::ProfileLoadFilterFn& filter_fn,
      const Options& options);

  DISALLOW_COPY_AND_ASSIGN(ProfileAssistant);
};

}  // namespace art

#endif  // ART_PROFMAN_PROFILE_ASSISTANT_H_