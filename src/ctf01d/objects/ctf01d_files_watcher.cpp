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

#include "ctf01d_files_watcher.h"
#include <wsjcpp_core.h>
#include <filesystem>

namespace ctf01d {

files_watcher::files_watcher() {
  TAG = "files_watcher";
}

bool files_watcher::watchFile(const std::string &filepath) {
  if (!WsjcppCore::fileExists(filepath)) {
    WsjcppLog::err(TAG, "File '" + filepath + "' did not found");
    return false;
  }
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_files.find(filepath);
  if (it != m_files.end()) {
    WsjcppLog::err(TAG, "File '" + filepath + "' already in watch list.");
    return false;
  }
  // set real time modified
  std::filesystem::file_time_type ftime = std::filesystem::last_write_time(filepath.c_str());
  m_files[filepath] = std::chrono::duration_cast<std::chrono::milliseconds>(ftime.time_since_epoch()).count();
  return true;
}

void files_watcher::stopWatchingFile(const std::string &filepath) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_files.find(filepath);
  if (it != m_files.end()) {
    m_files.erase(it);
  }
}

long files_watcher::getLastModifiedTimeFile(const std::string &filepath) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_files.find(filepath);
  if (it == m_files.end()) {
    return 0;
  }
  if (!WsjcppCore::fileExists(filepath)) {
    WsjcppLog::err(TAG, "File '" + filepath + "' did not found");
    return 0;
  }
  return m_files[filepath];
}

bool files_watcher::isModifiedFile(const std::string &filepath) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_files.find(filepath);
  if (it == m_files.end()) {
    return false;
  }
  if (!WsjcppCore::fileExists(filepath)) {
    WsjcppLog::err(TAG, "File '" + filepath + "' did not found");
    return false;
  }
  std::filesystem::file_time_type ftime = std::filesystem::last_write_time(filepath.c_str());
  long lastModifiedTime = std::chrono::duration_cast<std::chrono::milliseconds>(ftime.time_since_epoch()).count();
  if (m_files[filepath] != lastModifiedTime) {
    m_files[filepath] = lastModifiedTime;
    return true;
  }
  return false;
}

std::map<std::string, long> files_watcher::get_modified_files() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::map<std::string, long> ret;
  for (auto it = m_files.begin(); it != m_files.end(); ++it) {
    const std::string &filepath = it->first;
    if (!WsjcppCore::fileExists(filepath)) {
      WsjcppLog::err(TAG, "File '" + filepath + "' did not found");
      // don't remove from m_files
      continue;
    }
    std::filesystem::file_time_type ftime = std::filesystem::last_write_time(filepath.c_str());
    long lastModifiedTime = std::chrono::duration_cast<std::chrono::milliseconds>(ftime.time_since_epoch()).count();
    if (m_files[filepath] != lastModifiedTime) {
      m_files[filepath] = lastModifiedTime;
      ret[filepath] = m_files[filepath];
    }
  }
  return ret;
}

} // namespace ctf01d
