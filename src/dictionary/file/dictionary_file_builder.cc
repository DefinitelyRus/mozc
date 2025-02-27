// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dictionary/file/dictionary_file_builder.h"

#include <ios>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "base/file_stream.h"
#include "base/logging.h"
#include "dictionary/file/codec_interface.h"
#include "dictionary/file/section.h"

namespace mozc {
namespace dictionary {

DictionaryFileBuilder::DictionaryFileBuilder(
    DictionaryFileCodecInterface *file_codec)
    : file_codec_(file_codec) {
  DCHECK(file_codec_);
}

DictionaryFileBuilder::~DictionaryFileBuilder() {
  for (std::vector<DictionaryFileSection>::iterator itr = sections_.begin();
       itr != sections_.end(); itr++) {
    delete[] itr->ptr;
  }
}

bool DictionaryFileBuilder::AddSectionFromFile(
    const absl::string_view section_name, const std::string &file_name) {
  if (added_.find(section_name) != added_.end()) {
    DLOG(INFO) << "Already added: " << section_name;
    return false;
  }
  added_.emplace(section_name);

  InputFileStream ifs(file_name, std::ios::binary);
  CHECK(ifs) << "Failed to read" << file_name;

  ifs.seekg(0, std::ios::end);
  const int len = ifs.tellg();

  // Reads the file
  ifs.seekg(0, std::ios::beg);
  char *ptr = new char[len];
  ifs.read(ptr, len);

  sections_.push_back(DictionaryFileSection(ptr, len, ""));
  sections_.back().name = file_codec_->GetSectionName(section_name);
  return true;
}

void DictionaryFileBuilder::WriteImageToFile(
    const std::string &file_name) const {
  LOG(INFO) << "Start writing dictionary file to " << file_name;
  OutputFileStream ofs(file_name, std::ios::binary);
  file_codec_->WriteSections(sections_, &ofs);
  LOG(INFO) << "Generated";
}

}  // namespace dictionary
}  // namespace mozc
