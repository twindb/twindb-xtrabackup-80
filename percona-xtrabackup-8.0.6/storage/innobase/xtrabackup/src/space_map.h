/******************************************************
Copyright (c) 2018-2019 Percona LLC and/or its affiliates.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*******************************************************/

#ifndef XTRABACKUP_SPACE_MAP_H
#define XTRABACKUP_SPACE_MAP_H

#include <rapidjson/fwd.h>
#include <map>
#include <string>
#include <vector>

#include "backup_mysql.h"
#include "datasink.h"

/* Tablespace to file name mapping */
class Tablespace_map {
 public:
  enum tablespace_type_t { TABLESPACE = 1, UNDO_LOG };
  struct tablespace_t {
    std::string file_name;
    std::string name;
    tablespace_type_t type;
    tablespace_t() : file_name(), name(), type() {}
    tablespace_t(const tablespace_t &) = default;
    tablespace_t(std::string file_name, std::string name,
                 tablespace_type_t type)
        : file_name(file_name), name(name), type(type) {}
    tablespace_t &operator=(const tablespace_t &) = default;
  };
  typedef std::unordered_map<std::string, tablespace_t> map_t;
  typedef std::vector<tablespace_t> vector_t;

 private:
  map_t file_by_space;
  map_t space_by_file;

  /** Serialize talespace list into a buffer.
  @param[out]  buf output buffer */
  bool serialize(rapidjson::StringBuffer &buf) const;

  Tablespace_map(){}

  static Tablespace_map static_tablespace_map;

 public:
  /** Scan I_S.FILES for extended tablespaces.
  @param[in]  connection MySQL connection object */
  void scan(MYSQL *connection);

  /** Serialize talespace list into datasink.
  @param[in]  ds datasink */
  bool serialize(ds_ctxt_t *ds) const;

  /** Serialize talespace list into the current directory. */
  bool serialize() const;

  /** Deserialize talespace list from given directory.
  @param[in]  dir directory to read file from */
  bool deserialize(const std::string &dir);

  /** Add tablespace to the list.
  @param[in]  tablespace tablespace object */
  void add(const tablespace_t &tablespace);

  /** Remove tablepsace from the list.
  @param[in]  tablespace_name tablespace name */
  void erase(const std::string &tablespace_name);

  /** Return file name in the backup directory for given tablespace.
  @param[in]  file_name source tablespace file name */
  std::string backup_file_name(const std::string &file_name) const;

  /** Return original file name for given tablespace.
  @param[in]  space_name source tablespace name */
  std::string external_file_name(const std::string &space_name) const;

  /** Return the list of external tablespaces. */
  vector_t external_files() const;

  /** Return the list of external tablespaces. */
  static Tablespace_map &instance();
};

#endif /* XTRABACKUP_SPACE_MAP_H */
