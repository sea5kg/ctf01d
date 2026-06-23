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

#include <wsjcpp_employees.h>
#include <wsjcpp_yaml.h>
#include <wsjcpp_core.h>
#include <sstream>
#include <thread>
#include <ctime>
#include <locale>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "ctf01d/objects/ctf01d_files_watcher.h"
#include "ctf01d/objects/ctf01d_game_config.h"
#include <sea5kg_logger.h>
#include "ctf01d/include/ctf01d_images.h"
#include "ctf01d/include/ctf01d_globals.h"
#include "ctf01d/include/ctf01d_config.h"
#include "ctf01d/include/ctf01d_web_server.h"
#include "third_party/smallsha1/smallsha1.h"
#include "third_party/HowardHinnant/date.h"
#include <sys/stat.h>
#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

class employ_config : public WsjcppEmployBase, public ctf01d::config {
public:
  employ_config();
  ~employ_config();
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // ctf01d::config
  virtual void set_ctf01d_version(const std::string &ctf01d_version) override;
  virtual std::string ctf01d_version() override;
  virtual void set_work_dir(const std::string &work_dir) override;
  virtual std::string get_work_dir() override;
  virtual bool apply_config() override;
  virtual const std::vector<ctf01d::service_config> &services() override;
  virtual const std::vector<ctf01d::team_config> &teams() override;
  virtual bool contains_team_id(const std::string &team_id) const override;
  virtual std::string find_team_id_by_subnet(const std::string &ip) const override;
  virtual int scoreboard_port() const override;
  virtual std::string scoreboard_html_folder() const override;
  virtual std::shared_ptr<ctf01d::var_bool> scoreboard_auto_detection_team_id_by_subnet_ip() const override;
  virtual bool scoreboard_random() const override;
  virtual std::shared_ptr<ctf01d::var_bool> scoreboard_metrics_enabled() const override;
  virtual std::shared_ptr<ctf01d::var_allowed_ip> scoreboard_metrics_allowed_for() const override;
  virtual std::string game_id() const override;
  virtual std::string game_name() const override;
  virtual int flag_lifetime_in_seconds() const override;
  virtual std::shared_ptr<ctf01d::var_int> get_flag_cost_in_points() const override;
  virtual int game_start_utc_in_seconds() const override;
  virtual int game_end_utc_in_seconds() const override;
  virtual bool game_has_coffee_break() const override;
  virtual int game_coffee_break_start_utc_in_seconds() const override;
  virtual int game_coffee_break_end_utc_in_seconds() const override;
  virtual std::shared_ptr<ctf01d::flag_id_generator> default_flag_id_generator() override;

private:
  void update_files_in_data();
  bool update_ssl_keys();
  nlohmann::json load_files_sha1();
  void save_files_sha1(nlohmann::json &files);
  void update_data_html(nlohmann::json &files);

  bool checkYamlMainKeys(WsjcppYaml &yaml);
  bool applyScoreboardPortFromEnv();
  bool applyServicesConfig(WsjcppYaml &yaml);
  bool readTeamsConf(WsjcppYaml &yaml);
  bool init_work_dir();
  bool init_logger();

  void thread_watcher();
  void hot_reload_config_yaml();

  std::string TAG;
  std::string m_ctf01d_version;
  std::string m_work_dir;
  std::string m_config_filepath;
  bool m_applied_config;

  // scoreboard config
  ctf01d::scope_vars m_scoreboard_vars = ctf01d::scope_vars("scoreboard_config");
  std::shared_ptr<ctf01d::var_int> m_scoreboard_port;
  std::shared_ptr<ctf01d::var_dir> m_scoreboard_html_folder;
  std::shared_ptr<ctf01d::var_bool> m_scoreboard_auto_detection_team_id_by_subnet_ip;
  std::shared_ptr<ctf01d::var_bool> m_scoreboard_random;
  std::shared_ptr<ctf01d::var_bool> m_scoreboard_metrics_enabled;
  std::shared_ptr<ctf01d::var_allowed_ip> m_scoreboard_metrics_allowed_for;

  // game config
  ctf01d::game_config m_game_config;

  ctf01d::scope_vars m_teams_config_vars = ctf01d::scope_vars("teams_config");
  std::shared_ptr<ctf01d::var_string> m_ip_or_host_prefix;
  std::shared_ptr<ctf01d::var_string> m_ip_or_host_suffix;

  // teams config
  std::vector<ctf01d::team_config> m_teams;
  std::map<std::string, ctf01d::team_config> m_teams_cache;
  std::vector<std::string> m_teams_ip_or_host;
  std::map<std::string, std::string> m_teams_subnets;

  // services config
  std::vector<ctf01d::service_config> m_services;

  // hot-reload: for reload config in runtime
  std::thread m_thread_watcher;
  std::mutex m_mutex_thread_watcher;
  std::shared_ptr<ctf01d::files_watcher> m_files_watcher;
};

REGISTRY_WSJCPP_EMPLOY(employ_config)

employ_config::employ_config()
: WsjcppEmployBase({ ctf01d::config::name() }, {}) {
  TAG = ctf01d::config::name();
  m_ctf01d_version = "v??";
  m_files_watcher = std::make_shared<ctf01d::files_watcher>();

  // game options
  m_applied_config = false;

  // scoreboard config
  m_scoreboard_port = ctf01d::var_int::create({"scoreboard", "port"}, 8080, m_scoreboard_vars);
  m_scoreboard_port->set_minimum(ctf01d::MIN_TCP_PORT);
  m_scoreboard_port->set_maximum(ctf01d::MAX_TCP_PORT);
  m_scoreboard_random = ctf01d::var_bool::create({"scoreboard", "random"}, false, m_scoreboard_vars);
  m_scoreboard_html_folder = ctf01d::var_dir::create({"scoreboard", "html-dir-path"}, "./html", m_work_dir, m_scoreboard_vars);
  m_scoreboard_auto_detection_team_id_by_subnet_ip = ctf01d::var_bool::create({"scoreboard", "auto-detection-team-id-by-subnet-ip"}, "", m_scoreboard_vars);
  m_scoreboard_metrics_enabled = ctf01d::var_bool::create({"scoreboard", "prometheus-metrics-endpoint", "enabled"}, false, m_scoreboard_vars);
  m_scoreboard_metrics_allowed_for = ctf01d::var_allowed_ip::create({"scoreboard", "prometheus-metrics-endpoint", "allowed-for"}, "127.0.*", m_scoreboard_vars);

  m_ip_or_host_prefix = ctf01d::var_string::create({"config", "ip-or-host-prefix"}, "", m_teams_config_vars);
  m_ip_or_host_suffix = ctf01d::var_string::create({"config", "ip-or-host-suffix"}, "", m_teams_config_vars);
}

employ_config::~employ_config() {
  // TODO cleanup
  m_scoreboard_vars.clear();
}

bool employ_config::init(const std::string &sName, bool bSilent) {
  if (!init_work_dir()) {
    return false;
  }

  if (!init_logger()) {
    return false;
  }

  this->update_files_in_data();

  this->update_ssl_keys();

  if (!this->apply_config()) {
    sea5kg::log::err(TAG, "Configuration file has some problems");
    return false;
  }

  return true;
}

bool employ_config::deinit(const std::string &sName, bool bSilent) {
  sea5kg::log::info(TAG, "deinit");
  // wait stop threads
  if (m_thread_watcher.joinable()) {
    m_thread_watcher.join();
  }
  return true;
}

void employ_config::set_ctf01d_version(const std::string &ctf01d_version) {
  m_ctf01d_version = ctf01d_version;
}

std::string employ_config::ctf01d_version() {
  return m_ctf01d_version;
}

void employ_config::set_work_dir(const std::string &sWorkDir) {
  if (m_work_dir != "" && m_work_dir != sWorkDir) {
    std::cout << "Changed work-dir to '" + sWorkDir + "'" << std::endl;
  }
  m_work_dir = sWorkDir;
  m_config_filepath = m_work_dir + "/config.yml";
  m_scoreboard_html_folder->set_root_dir(m_work_dir);
}

std::string employ_config::get_work_dir() {
    return m_work_dir;
}

bool employ_config::apply_config() {
  if (m_applied_config) {
    return true;
  }

  m_applied_config = false;
  sea5kg::log::info(TAG, "Loading configuration...");

  sea5kg::log::info(TAG, "Reading config: " + m_config_filepath);

  if (!WsjcppCore::fileExists(m_config_filepath)) {
    sea5kg::log::err(TAG, "File " + m_config_filepath + " does not exists");
    return false;
  }

  WsjcppYaml yaml;
  std::string sError;
  if (!yaml.loadFromFile(m_config_filepath, sError)) {
    sea5kg::log::err(TAG, "Could not parse " + m_config_filepath + ", reason: " + sError);
    return false;
  }

  if (!checkYamlMainKeys(yaml)) {
    return false;
  }

  auto cursor = yaml.getCursor();
  std::string err;
  if (!m_game_config.read(cursor, m_work_dir, err)) {
    sea5kg::log::err(TAG, err);
    return false;
  }

  if (!m_scoreboard_vars.read(cursor, err)) {
    sea5kg::log::err(TAG, err);
    return false;
  }

  // CTF01D_PORT
  if (!applyScoreboardPortFromEnv()) {
    return false;
  }

  if (!this->applyServicesConfig(yaml)) {
    return false;
  }

  if (!this->readTeamsConf(yaml)) {
    return false;
  }

  m_applied_config = true;
  m_files_watcher->watchFile(m_config_filepath);
  m_thread_watcher = std::thread(&employ_config::thread_watcher, this);
  return m_applied_config;
}

const std::vector<ctf01d::team_config> &employ_config::teams() {
  return m_teams;
}

bool employ_config::contains_team_id(const std::string &team_id) const {
  return m_teams_cache.count(team_id) != 0;
}

std::string employ_config::find_team_id_by_subnet(const std::string &ip) const {
  std::string ip_subnet = ip.substr(0, ip.rfind('.'));
  if (m_teams_subnets.count(ip_subnet) > 0) {
    return m_teams_subnets.at(ip_subnet);
  }
  return "";
}

const std::vector<ctf01d::service_config> &employ_config::services() {
  return m_services;
}

int employ_config::scoreboard_port() const {
  return m_scoreboard_port->value();
}

std::string employ_config::scoreboard_html_folder() const {
  return m_scoreboard_html_folder->value();
}

std::shared_ptr<ctf01d::var_bool> employ_config::scoreboard_auto_detection_team_id_by_subnet_ip() const {
  return m_scoreboard_auto_detection_team_id_by_subnet_ip;
}

bool employ_config::scoreboard_random() const {
  return m_scoreboard_random->value();
}

std::shared_ptr<ctf01d::var_bool> employ_config::scoreboard_metrics_enabled() const {
  return m_scoreboard_metrics_enabled;
}

std::shared_ptr<ctf01d::var_allowed_ip> employ_config::scoreboard_metrics_allowed_for() const {
  return m_scoreboard_metrics_allowed_for;
}

std::string employ_config::game_id() const {
  return m_game_config.id();
}

std::string employ_config::game_name() const  {
  return m_game_config.name();
}

int employ_config::flag_lifetime_in_seconds() const  {
  return m_game_config.flag_lifetime_in_seconds();
}

std::shared_ptr<ctf01d::var_int> employ_config::get_flag_cost_in_points() const {
  return m_game_config.flag_cost_in_points();
}

int employ_config::game_start_utc_in_seconds() const {
  // TODO return var
  return m_game_config.start_utc_in_seconds();
}

int employ_config::game_end_utc_in_seconds() const {
  // TODO return var
  return m_game_config.end_utc_in_seconds();
}

bool employ_config::game_has_coffee_break() const {
  return m_game_config.has_coffee_break();
}

int employ_config::game_coffee_break_start_utc_in_seconds() const {
  return m_game_config.coffee_break_start_utc_in_seconds();
}

int employ_config::game_coffee_break_end_utc_in_seconds() const {
  return m_game_config.coffee_break_end_utc_in_seconds();
}

std::shared_ptr<ctf01d::flag_id_generator> employ_config::default_flag_id_generator() {
  return m_game_config.default_flag_id_generator();
}

// helper
std::string sha1_by_string(const std::string &data) {
  char hexstring[41]; // 40 chars + a zero
  std::memset(hexstring, 0, sizeof hexstring);

  unsigned char hash[20];
  sha1::calc(data.c_str(), data.length(), hash);
  sha1::toHexString(hash, hexstring);
  return std::string(hexstring);
}

std::string sha1_by_data(const char *data, int len) {
  char hexstring[41]; // 40 chars + a zero
  std::memset(hexstring, 0, sizeof hexstring);

  unsigned char hash[20];
  sha1::calc(data, len, hash);
  sha1::toHexString(hash, hexstring);
  return std::string(hexstring);
}

std::string sha1_by_file(const std::string &sFilename) {
  std::ifstream f(sFilename, std::ifstream::binary);
  if (!f) {
    return "Could not open file";
  }
  // get length of file:
  f.seekg (0, f.end);
  int nBufferSize = f.tellg();
  f.seekg (0, f.beg);
  char *pBuffer = new char [nBufferSize];
  // read data as a block:
  f.read(pBuffer, nBufferSize);
  if (!f) {
    delete[] pBuffer;
    // f.close();
    sea5kg::log::throw_err("sha1_by_file", "Could not read file. Only " + std::to_string(f.gcount()) + " could be read");
    return "";
  }
  f.close();
  char hexstring[41]; // 40 chars + a zero
  std::memset(hexstring, 0, sizeof hexstring);
  unsigned char hash[20];
  sha1::calc(pBuffer, nBufferSize, hash);
  sha1::toHexString(hash, hexstring);
  delete[] pBuffer;
  return std::string(hexstring);
}

bool employ_config::update_ssl_keys() {
  std::string error;
  std::string data_keys_dir = m_work_dir + "/keys";
  if (!WsjcppCore::dirExists(data_keys_dir)) {
    WsjcppCore::makeDir(data_keys_dir);
    if (!WsjcppCore::setFilePermissions(data_keys_dir, WsjcppFilePermissions(0x755), error)) {
      sea5kg::log::throw_err(TAG, error);
    }
  }
  std::string flag_private_path = data_keys_dir + "/auto_flag_private";
  std::string flag_public_path = data_keys_dir + "/auto_flag_public.pub";

  int bits = 2048;

  if (!WsjcppCore::fileExists(flag_private_path) || !WsjcppCore::fileExists(flag_public_path)) {
    // 1. Инициализируем генератор случайных чисел
    RAND_poll();
    
    // 2. Генерируем RSA ключ
    RSA* rsa = RSA_new();
    BIGNUM* exp = BN_new();
    BN_set_word(exp, RSA_F4); // 65537 - стандартная экспонента
    
    if (!RSA_generate_key_ex(rsa, bits, exp, nullptr)) {
        std::cerr << "Ошибка генерации ключа!" << std::endl;
        return false;
    }

    BIO* priv_bio = BIO_new_file(flag_private_path.c_str(), "w");
    if (!PEM_write_bio_RSAPrivateKey(priv_bio, rsa, nullptr, nullptr, 0, nullptr, nullptr)) {
        std::cerr << "Ошибка сохранения приватного ключа!" << std::endl;
        return false;
    }
    BIO_free(priv_bio);
    
    BIO* pub_bio = BIO_new_file(flag_public_path.c_str(), "w");
    if (!PEM_write_bio_RSA_PUBKEY(pub_bio, rsa)) {
        std::cerr << "Ошибка сохранения публичного ключа!" << std::endl;
        return false;
    }
    BIO_free(pub_bio);
    
    // 5. Очистка
    RSA_free(rsa);
    BN_free(exp);
    
    std::cout << "✓ Ключи сгенерированы!" << std::endl;
  }
  return true;
}

void employ_config::update_files_in_data() {
  std::string sError;
  if (!WsjcppCore::dirExists(m_work_dir + "/logs")) {
    WsjcppCore::makeDir(m_work_dir + "/logs");
    if (!WsjcppCore::setFilePermissions(m_work_dir + "/logs", WsjcppFilePermissions(0x755), sError)) {
      sea5kg::log::throw_err(TAG, sError);
    }
  }

  nlohmann::json previous_files_sha1 = load_files_sha1();

  if (!WsjcppCore::fileExists(m_work_dir + "/config.yml")) {
    sea5kg::log::warn(TAG, "Extracting config.yml and files");
    sea5kg::log::warn(TAG, "Extracting checker_example_*");
    const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
    std::vector<std::string> vExecutableFiles;
    for (int i = 0; i < vFiles.size(); i++) {
      std::string filepath = vFiles[i]->getFilename();
      if (filepath.rfind("./data_sample/checker_example_", 0) == 0) {
        std::vector<std::string> vPath = WsjcppCore::split(filepath, "/");
        std::string sDirname = vPath[2];
        vPath.erase (vPath.begin(),vPath.begin()+3);
        std::string sNewFilepath = WsjcppCore::join(vPath, "/");
        sNewFilepath = wsjcpp::normalizeFilePath(m_work_dir + "/" + sDirname + "/" + sNewFilepath);
        if (!WsjcppCore::fileExists(sNewFilepath)) {
          std::cout << "Extracting file '" << filepath << "' to '" << sNewFilepath << "'" << std::endl;
        } else {
          std::cout << "File '" << sNewFilepath << "' already exists. Skip." << std::endl;
          continue;
        }

        // prepare folder
        std::string sFolder = wsjcpp::normalizeFilePath(m_work_dir + "/" + sDirname + "/");
        if (!WsjcppCore::dirExists(sFolder)) {
          WsjcppCore::makeDir(sFolder);
        }

        if (!WsjcppCore::writeFile(sNewFilepath, vFiles[i]->getBuffer(), vFiles[i]->getBufferSize())) {
          std::cout << "ERROR. Could not write file. " << std::endl;
          continue;
        } else {
          std::cout << "Successfully created file. " << std::endl;
          // TODO redesign set permission via wsjcpp
          if (chmod(sNewFilepath.c_str(), S_IRWXU|S_IRWXG) != 0) {
            std::cout << "ERROR. Could not change permissions for. " << sNewFilepath << std::endl;
          } else {
            struct stat info;
            stat(sNewFilepath.c_str(), &info);
            printf("after chmod(), permissions are: %08x\n", info.st_mode);
          }
        }
      }
    }

    WsjcppResourceFile* pConfigYml = WsjcppResourcesManager::get("./data_sample/config.yml");
    std::string sNewFilepath = wsjcpp::normalizeFilePath(m_work_dir + "/config.yml");
    if (!WsjcppCore::writeFile(sNewFilepath, pConfigYml->getBuffer(), pConfigYml->getBufferSize())) {
      std::cout << "ERROR. Could not write file. " << std::endl;
    } else {
      std::cout << "Successfully created file. " << std::endl;
    }
  }

  update_data_html(previous_files_sha1);
  save_files_sha1(previous_files_sha1);
}

nlohmann::json employ_config::load_files_sha1() {
  nlohmann::json files_sha1;
  if (WsjcppCore::fileExists(m_work_dir + "/files_sha1.json")) {
    std::ifstream ifs(m_work_dir + "/files_sha1.json");
    files_sha1 = nlohmann::json::parse(ifs);
  }
  return files_sha1;
}

void employ_config::save_files_sha1(nlohmann::json &files) {
  std::ofstream output(m_work_dir + "/files_sha1.json");
  output << std::setw(2) << files << std::endl;
}

void employ_config::update_data_html(nlohmann::json &previous_files_sha1) {
  sea5kg::log::warn(TAG, "Updating files in data/html");
  if (!WsjcppCore::dirExists(m_work_dir + "/html")) {
    WsjcppCore::makeDir(m_work_dir + "/html");
  }
  
  const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
  for (int i = 0; i < vFiles.size(); i++) {
    std::string source_filepath = vFiles[i]->getFilename();
    if (source_filepath.rfind("./data_sample/html/", 0) != 0) {
      continue;
    }
    // remove base folder
    std::vector<std::string> vPath = WsjcppCore::split(source_filepath, "/");
    vPath.erase (vPath.begin(),vPath.begin()+3);
    std::string target_filepath = WsjcppCore::join(vPath, "/");
    target_filepath = wsjcpp::normalizeFilePath(m_work_dir + "/html/" + target_filepath);

    // prepare folders
    if (!WsjcppCore::fileExists(target_filepath)) {
      std::string dirpath = wsjcpp::normalizeFilePath(m_work_dir + "/html/");
      for (int p = 0; p < vPath.size()-1; p++) {
        dirpath = wsjcpp::normalizeFilePath(dirpath + "/" + vPath[p]);
        if (!WsjcppCore::dirExists(dirpath)) {
          if (!WsjcppCore::makeDir(dirpath)) {
            std::cout << "ERROR. Could not create: " << dirpath << std::endl;
            continue;
          }
        }
      }
    }

    std::string previous_sha1 = "";
    if (previous_files_sha1.contains(source_filepath)) {
      previous_sha1 = previous_files_sha1[source_filepath];
    }

    if (WsjcppCore::fileExists(target_filepath) && previous_sha1 != "") {
      if (previous_sha1 != sha1_by_file(target_filepath)) {
        // Skip. file has changes by user. Skip.
        std::cout << "Warning. Could not override file, because has changes: " << target_filepath << std::endl;
        continue;
      }
    }

    std::string new_sha1 = sha1_by_data(vFiles[i]->getBuffer(), vFiles[i]->getBufferSize());
    if (WsjcppCore::fileExists(target_filepath) && new_sha1 == previous_sha1) {
      // Skip. file has same content
      continue;
    }

    if (!WsjcppCore::writeFile(target_filepath, vFiles[i]->getBuffer(), vFiles[i]->getBufferSize())) {
      std::cout << "ERROR. Could not write/override file. " << std::endl;
      continue;
    }

    std::cout << "Successfully created/updated file: " << target_filepath << std::endl;
    std::string err;
    if (!WsjcppCore::setFilePermissions(target_filepath, WsjcppFilePermissions(0x644), err)) {
      sea5kg::log::throw_err(TAG, err);
    }
    previous_files_sha1[source_filepath] = new_sha1;
  }
}

bool employ_config::checkYamlMainKeys(WsjcppYaml &yaml) {
  // check main keys
  auto cur = yaml.getCursor();
  // TODO list from vars
  std::vector<std::string> expected_keys = {
    "scoreboard",
    "game",
    "services",
    "teams",
  };
  std::vector<std::string> main_keys = cur.keys();
  for (int i = 0; i < main_keys.size(); i++) {
    if (std::find(expected_keys.begin(), expected_keys.end(), main_keys[i]) == expected_keys.end()) {
      sea5kg::log::err(TAG, "Got unexpected key in main: '" + main_keys[i] + "'");
      return false;
    }
  }
  for (int i = 0; i < expected_keys.size(); i++) {
    if (std::find(main_keys.begin(), main_keys.end(), expected_keys[i]) == main_keys.end()) {
      sea5kg::log::err(TAG, "Not found expected key in config: '" + expected_keys[i] + "'");
      return false;
    }
  }
  return true;
}

bool employ_config::applyScoreboardPortFromEnv() {
  std::string str_port;
  if (WsjcppCore::getEnv("CTF01D_PORT", str_port)) {
    sea5kg::log::warn(TAG, "CTF01D_PORT='" + str_port + "'");
    try {
      int port = std::stoi(str_port);
      std::string err;
      if (!m_scoreboard_port->set_value(port, err)) {
        sea5kg::log::err(TAG, "CTF01D_PORT='" + str_port + "' is wrong. " + err);
        return false;
      }
    } catch (const std::invalid_argument& e) {
      sea5kg::log::err(TAG, "No conversion could be performed. CTF01D_PORT='" + str_port + "'");
      std::cerr << "Error: \n";
      return false;
    } catch (const std::out_of_range& e) {
      sea5kg::log::err(TAG, "The converted value is too big for an int.. CTF01D_PORT='" + str_port + "'");
      return false;
    } catch (...) {
      sea5kg::log::err(TAG, "The converted value is too big for an int.. CTF01D_PORT='" + str_port + "'");
      return false;
    }
    sea5kg::log::info(TAG, "scoreboard.port will be overridden from environment variable. CTF01D_PORT='" + str_port + "'");
    return true;
  }
  return true;
}

bool employ_config::applyServicesConfig(WsjcppYaml &yaml) {
  m_services.clear();
  auto images = findWsjcppEmploy<ctf01d::images>();

  WsjcppYamlCursor yamlCheckers = yaml["services"];

  if (yamlCheckers.size() == 0) {
    sea5kg::log::err(TAG, "Checkers does not defined");
    return false;
  }

  for (int i = 0; i < yamlCheckers.size(); i++) {
    WsjcppYamlCursor yamlChecker = yamlCheckers[i];

    // default values of service config
    ctf01d::service_config _serviceConf;
    std::string err;

    if (!_serviceConf.read(yamlChecker, m_work_dir, err)) {
      sea5kg::log::err(TAG, err);
      return false;
    }

    if (!_serviceConf.is_enabled()) {
      sea5kg::log::warn(TAG, "Checker for service " + _serviceConf.id() + " - disabled ");
      continue;
    }

    for (unsigned int i = 0; i < m_services.size(); i++) {
      if (m_services[i].id() == _serviceConf.id()) {
        sea5kg::log::err(TAG, "Already registered checker for service '" + _serviceConf.id() + "'");
        return false;
      }
    }

    if (!images->load_service_logo(_serviceConf.id(), _serviceConf.logo_path())) {
      return false;
    }
    sea5kg::log::info(TAG, "Loaded service logo = " + _serviceConf.logo_path());
    if (!images->load_service_big_logo(_serviceConf.id(), _serviceConf.logo_big_path())) {
      return false;
    }
    sea5kg::log::info(TAG, "Loaded service logo-big = " + _serviceConf.logo_big_path());

    m_services.push_back(_serviceConf);

    // set write permissions for all to directory with checker
    if (!WsjcppCore::setFilePermissions(_serviceConf.script_dir(), WsjcppFilePermissions(0x777), err)) {
      sea5kg::log::err(TAG, err);
      return false;
    }

    std::string script_absolute_path = wsjcpp::normalizeFilePath(_serviceConf.script_dir() + "/" + _serviceConf.script_path());
    if (!WsjcppCore::fileExists(script_absolute_path)) {
      sea5kg::log::err(TAG, "File " + script_absolute_path + " did not exists");
      return false;
    }
    // set write permissions for all to script of checker
    if (!WsjcppCore::setFilePermissions(script_absolute_path, WsjcppFilePermissions(0x777), err)) {
      sea5kg::log::err(TAG, err);
      return false;
    }

    sea5kg::log::ok(TAG, "Registered checker for service " + _serviceConf.id());
  }

  if (m_services.size() == 0) {
    sea5kg::log::err(TAG, "No one defined services in config");
    return false;
  }

  return true;
}

bool employ_config::readTeamsConf(WsjcppYaml &yaml) {
  m_teams.clear();
  m_teams_cache.clear();
  m_teams_ip_or_host.clear();
  m_teams_subnets.clear();
  auto images = findWsjcppEmploy<ctf01d::images>();

  WsjcppYamlCursor cursor = yaml["teams"];
  std::string err;
  if (!m_teams_config_vars.read(cursor, err)) {
    sea5kg::log::err(TAG, err);
    return false;
  }

  if (!cursor.hasKey("list")) {
    sea5kg::log::err(TAG, "Missing teams.list");
    return false;
  }
  cursor = cursor["list"];

  if (cursor.size() == 0) {
    sea5kg::log::err(TAG, "Teams does not defined");
    return false;
  }

  for (int i = 0; i < cursor.size(); i++) {
    WsjcppYamlCursor cur = cursor[i];
    ctf01d::team_config _team_config;
    _team_config.set_ip_or_host_prefix(m_ip_or_host_prefix->value());
    _team_config.set_ip_or_host_suffix(m_ip_or_host_suffix->value());

    std::string err;
    if (!_team_config.read(cur, m_work_dir, err)) {
      sea5kg::log::err(TAG, err);
      return false;
    }

    // TODO check sTeamId format
    if (!_team_config.is_active()) {
      sea5kg::log::warn(TAG, "Team " + _team_config.id() + " - deactivated");
      continue;
    }

    for (unsigned int i = 0; i < m_teams.size(); i++) {
      if (m_teams[i].id() == _team_config.id()) {
        sea5kg::log::err(TAG, "Already registered team with id " + _team_config.id());
        return false;
      }
    }

    // Check duplicate IP addresses
    if (std::find(m_teams_ip_or_host.begin(), m_teams_ip_or_host.end(), _team_config.ip_or_host()) == m_teams_ip_or_host.end()) {
      m_teams_ip_or_host.push_back(_team_config.ip_or_host());
    } else {
      sea5kg::log::err(TAG, "Found duplicate IP or Host address: " + _team_config.ip_or_host());
      return false;
    }

    if (!images->load_team_logo(_team_config.id(), _team_config.logo_path())) {
      return false;
    }
    sea5kg::log::info(TAG, "Loaded team logo = " + _team_config.logo_path());
    if (!images->load_team_big_logo(_team_config.id(), _team_config.logo_big_path())) {
      return false;
    }
    sea5kg::log::info(TAG, "Loaded team logo-big = " + _team_config.logo_path());

    m_teams.push_back(_team_config);
    m_teams_cache[_team_config.id()] = _team_config;
    
    if (m_scoreboard_auto_detection_team_id_by_subnet_ip->value()) {
      if (m_teams_subnets.count(_team_config.ip_subnet()) > 0) {
        sea5kg::log::err(TAG, "Found duplicate subnet: " + _team_config.ip_subnet());
        return false;
      }
    }
    m_teams_subnets[_team_config.ip_subnet()] = _team_config.id();
    sea5kg::log::ok(TAG, "Registered team " + _team_config.id());
  }

  if (m_teams.size() == 0) {
    sea5kg::log::err(TAG, "No one defined team in config");
    return false;
  }

  return true;
}

bool employ_config::init_work_dir() {
  sea5kg::log::info(TAG, "Work Directory is " + m_work_dir);
  std::string sWorkDir = this->get_work_dir();
  if (sWorkDir == "") {
    sea5kg::log::throw_err(TAG, "Work Directory not defined.");
    return false;
  }
  if (!WsjcppCore::dirExists(sWorkDir)) {
    sea5kg::log::err(TAG, "Directory " + sWorkDir + " does not exists");
    return false;
  }
  return true;
}

bool employ_config::init_logger() {
  // init logger
  std::string sLogDir = m_work_dir + "/logs/" + WsjcppCore::getCurrentTimeForFilename();
  sLogDir = wsjcpp::normalizeFilePath(sLogDir);
  if (!WsjcppCore::dirExists(sLogDir)) {
    if (!WsjcppCore::makeDirsPath(sLogDir)) {
      sea5kg::log::err(TAG, "Could not make dirs for logs: " + sLogDir);
      return false;
    }
    std::string sError;
    if (!WsjcppCore::setFilePermissions(sLogDir, WsjcppFilePermissions(0x776), sError)) {
      sea5kg::log::throw_err(TAG, sError);
    }
  }
  if (!WsjcppCore::dirExists(sLogDir)) {
    std::cout << "Error: Folder '" << sLogDir << "' does not exists and could not created, please check access rights to parent folder.\n";
    return false;
  }
  sea5kg::log::set_log_filename_prefix("ctf01d");
  sea5kg::log::set_log_dirpath(sLogDir);
  sea5kg::log::set_rotation_period_in_seconds(600); // every 10 min  // TODO rotation period must be in config.yml
  sea5kg::log::set_enable_log_file(true);
  std::cout << "Logger: '" + sLogDir + "' \n";
  return true;
}

void employ_config::thread_watcher() {

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::map<std::string, long> modified_files = m_files_watcher->get_modified_files();
    if (modified_files.size() == 0) { // nothing changes
      continue;
    }
    sea5kg::log::info(TAG, "Watcher thread found changes");

    // TODO images/logos update

    std::scoped_lock lock(m_mutex_thread_watcher);

    for (auto it = modified_files.begin(); it != modified_files.end(); ++it) {
      const std::string &filepath = it->first;
      if (filepath == m_config_filepath) {
        hot_reload_config_yaml();
      } else {
        sea5kg::log::warn(TAG, "TODO update file watched " + filepath);
      }
    }
  }
}

void employ_config::hot_reload_config_yaml() {
  if (!WsjcppCore::fileExists(m_config_filepath)) {
    sea5kg::log::err(TAG, "File " + m_config_filepath + " does not exists");
    return;
  }
  WsjcppYaml yaml;
  std::string err;
  if (!yaml.loadFromFile(m_config_filepath, err)) {
    sea5kg::log::err(TAG, "Could not parse " + m_config_filepath + ", reason: " + err);
    return;
  }
  auto cursor = yaml.getCursor();
  {
    std::shared_ptr<ctf01d::var_bool> reload_bool_var;
    bool prev_value;

    reload_bool_var = m_scoreboard_metrics_enabled;
    prev_value = reload_bool_var->value();
    if (reload_bool_var->read(cursor, err)) {
      if (prev_value != reload_bool_var->value()) {
        sea5kg::log::info(TAG, "Updated option: " + reload_bool_var->name() + " " + reload_bool_var->to_string());
        findWsjcppEmploy<ctf01d::web_server>()->set_metrics_enabled(reload_bool_var->value());
      }
    };

    // TODO: before apply option need test subnets
    reload_bool_var = m_scoreboard_auto_detection_team_id_by_subnet_ip;
    prev_value = reload_bool_var->value();
    if (reload_bool_var->read(cursor, err)) {
      if (prev_value != reload_bool_var->value()) {
        sea5kg::log::info(TAG, "Updated option: " + reload_bool_var->name() + " " + reload_bool_var->to_string());
        findWsjcppEmploy<ctf01d::web_server>()->set_auto_detection_team_id_by_subnet_ip(reload_bool_var->value());
      }
    };
  }

  // std::shared_ptr<ctf01d::var_allowed_ip> m_scoreboard_metrics_allowed_for;

  // if (!m_scoreboard_vars.read(cursor, err)) {
  //   sea5kg::log::err(TAG, err);
  //   return;
  // }
}
