/**********************************************************************************
 *           Project
 *   _______ _________ _______  _______  __    ______
 *  (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
 *  | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
 *  | |         | |   | (__    | | /   |  | | | |   ) |
 *  | |         | |   |  __)   | (/ /) |  | | | |   | |
 *  | |         | |   | (      |   / | |  | | | |   ) |
 *  | (____/\   | |   | )      |  (__) |__) (_| (__/  )
 *  (_______/   )_(   |/       (_______)\____/(______/
 *
 * MIT License
 *
 * Copyright (c) 2018-2026 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Original repository: https://github.com/sea5kg/ctf01d
 *
 ***********************************************************************************/

#include "ctf01d_image.h"
#include <wsjcpp_core.h>
#include <filesystem>

namespace ctf01d {

image::image(const std::string &id)
: TAG("ctf01d::image"),
  m_id(id),
  m_last_modified_time(0)
{
  pBuffer = nullptr;
  nBufferSize = 0;
}

image::~image() {
  if (pBuffer != nullptr) {
    nBufferSize = 0;
    delete pBuffer;
  }
}

bool image::reload_from_file(const std::string &filepath) {

  WsjcppLog::info(TAG, "reload_from_file point 1");
  std::filesystem::file_time_type ftime = std::filesystem::last_write_time(filepath.c_str());
  long last_modified_time = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
  if (m_last_modified_time == last_modified_time) {
    // file has not changes
    return true;
  }
  WsjcppLog::info(TAG, "reload_from_file point 2");
  // cleanup buffer
  if (pBuffer != nullptr) {
    nBufferSize = 0;
    delete pBuffer;
  }

  WsjcppLog::info(TAG, "reload_from_file point 3");

  m_filepath = filepath;
  m_filename = WsjcppCore::extractFilename(filepath);
  if (!WsjcppCore::readFileToBuffer(m_filepath, &pBuffer, nBufferSize)) {
    return false;
  }
  WsjcppLog::info(TAG, "reload_from_file point 4");
  m_last_modified_time = last_modified_time;
  return true;
}

std::string image::id() {
  return m_id;
}

std::string image::filename() {
  return m_filename;
}

std::string image::filepath() {
  return m_filepath;
}

long image::last_modified_time() {
  return m_last_modified_time;
}

} // namespace ctf01d