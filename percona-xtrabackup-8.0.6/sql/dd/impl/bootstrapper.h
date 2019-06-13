/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef DD__BOOTSTRAPPER_INCLUDED
#define DD__BOOTSTRAPPER_INCLUDED

#include <sys/types.h>

#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/handler.h"         // dict_init_mode_t

class THD;

/**
  Create a lex string for the query from the string supplied
  and execute the query.

  @param thd     Thread handle.
  @param q_buf   String containing the query text.

  @retval false  Success.
  @retval true   Error.
*/
bool execute_query(THD *thd, const dd::String_type &q_buf);

/**
  Data dictionary initialization.

  The data dictionary is initialized whenever the mysqld process starts.
  We distinguish between the first time start and the subsequent normal
  restarts/upgrades, as explained below. However, there are three main
  design principles that should be elaborated first.

  1. Two-step process: The dictionary initialization is implemented as
     a two step process. First, scaffolding is built to prepare the
     synchronization with persistent storage, then, the actual synchronization
     is done. The way this is done depends on the context, and is different
     for first time start and the subsequent restarts.

  2. Use SQL: The initialization uses SQL to build the scaffolding. This
     means that we execute SQL statements to create the dictionary tables.
     Since this is done at a stage where the physical tables either do not
     exist yet, or are not known, we must instrument the DDL execution to
     create the physical counterpart of the tables only on first time start.
     The goal is to keep the instrumentation at a minimum.

  3. Fake caching: As a consequence of keeping instrumentation at a minimum,
     we provide uniform behavior of the caching layer in the data dictionary
     also in the scaffolding phase. This means that as seen from the outside,
     dictionary objects can be retrieved from the cache. Internally, below the
     caching layer, the objects are only kept in a separate buffer until all
     the required scaffolding is built. At that point, we can start using the
     underlying physical tables, depending on the circumstances:

     - For first time start (initialization), we can flush the meta data
       generated in the scaffolding phase, to the DD tables.
     - For ordinary restart, we can use the scaffolding to open the physical
       tables, and then sync up the real meta data that is stored persistently.
     - For upgrade, we first build scaffolding based on the actual DD tables,
       then we create the target DD tables, migrate the meta data from the old
       to the new tables, and finally switch from old to new tables
       atomically by means of DML on the DD tables. This means that we update
       the schema ids in the DD tables directly instead of executing
       'RENAME TABLE', which would do auto commit and thus break atomicity.

     After the scaffolding has been flushed or synced, what should be left is
     a collection of the core DD meta data objects. This collection is located
     in the storage adapter, and allows the DD cache to evict core DD objects
     in the same way as other DD objects.

  Please note that dictionary initialization is only a small part of server
  initialization. There is a lot going on before and after dictionary
  initialization while starting the server.

  Please see more elaborated descriptions for the initialize() and restart()
  methods below.
*/

namespace dd {
class Dictionary_impl;

namespace bootstrap {

/**
  Initialize the dictionary while starting the server for the first time.

  At this point, the DDSE has been initialized as a normal plugin. The
  dictionary initialization proceeds as follows:

   1. Preparation phase

  1.1 Call dict_init() to initialize the DDSE. This will make the predefined
      tablespaces be created physically, and their meta data be returned to
      the SQL layer along with the meta data for the DD tables required by
      the DDSE. The tables are not yet created physically.
  1.2 Prepare the dd::Tablespace objects reflecting the predefined tablespace
      objects and add them to the core registry in the storage adapter.

  2. Scaffolding phase

  2.1 Create and use the dictionary schema by executing SQL statements.
      The schema is created physically since this is the first time start,
      and the meta data is generated and stored in the core registry of
      the storage adapter without being written to disk.
  2.2 Create tables by executing SQL statements. Like for the schema, the
      tables are created physically, and the meta data is generated
      and stored in the core registry without being written to disk.
      This is done to prepare enough meta data to actually be able to
      open the DD tables.

  3. Synchronization phase

  3.1 Store meta data for the DD schema, tablespace and tables, i.e., the DD
      objects that were generated in the scaffolding phase, and make sure the
      IDs are maintained when the objects are stored.
  3.2 Populate the DD tables which have some predefined static contents to
      be inserted. This is, e.g., relevant for the 'catalogs' table, which
      only has a single default entry in it. Dynamic contents is added in
      other ways, e.g. by storing generated DD objects (see above) or by
      inserting data from other sources (see re-population of character sets
      in the context of server restart below).
  3.3 Store various properties of the DD tables, including the SE private data,
      a representation of the DDL statement used to create the table etc.
  3.4 Verify that the dictionary objects representing the core DD table meta
      data are present in the core registry of the storage adapter. If an
      object representing the meta data of a core DD table is not available,
      then we loose access to the DD tables, and we will not be able to handle
      cache misses or updates to the meta data.
  3.5 Update the version numbers that are stored, e.g. the DD version and the
      current mysqld server version.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/

bool initialize(THD *thd);

/**
  Initialize the dictionary while restarting the server.

  At this point, the DDSE has been initialized as a normal plugin. The
  dictionary initialization proceeds as follows:

  1. Preparation phase

  1.1 Call dict_init() to initialize the DDSE. This will retrieve the meta data
      of the predefined tablespaces and the DD tables required by the DDSE.
      Both the tables and the tablespaces are already created physically, the
      point here is just to get hold of enough meta data to start using the DD.
  1.2 Prepare the dd::Tablespace objects reflecting the predefined tablespace
      objects and add them to the core registry in the storage adapter.

  2. Scaffolding phase

  2.1 Create and use the dictionary schema by executing SQL statements.
      The schema is not created physically, but the meta data is generated
      and stored in the core registry without being written to disk.
  2.2 Create tables by executing SQL statements. Like for the schema, the
      tables are not created physically, but the meta data is generated
      and stored in the core registry without being written to disk.
      This is done to prepare enough meta data to actually be able to
      open the DD tables. The SQL DDL statements are either retrieved from
      the table definitions that are part of the server binary (for restart),
      or from one of the DD tables (for upgrade).

  3. Synchronization phase

  3.1 Read meta data for the DD tables from the DD tables. Here, we use the
      meta data from the scaffolding phase for the schema, tablespace and the
      DD tables to open the physical DD tables. We read the stored objects,
      and update the in-memory copies in the core registry with the real meta
      data from the objects that are retrieved form persistent storage. Finally,
      we flush the tables to empty the table definition cache to make sure the
      table share structures for the DD tables are re-created based on the
      actual meta data that was read from disk rather than the temporary meta
      data from the scaffolding phase.
  3.2 If this is a restart with a new DD version, we must upgrade the DD
      tables. In that case, we create the new target DD tables in a temporary
      schema, migrate the meta data to the new tables, and then do DML on the
      DD tables to make sure the new DD tables will be used instead of the old
      ones. This DML involves changing the schema ids directly in the DD tables,
      and updating the meta data stored in the 'dd_properties' DD table.
      This will make sure the switch from the old to the new tables is
      atomic. After this is done, we will reset the DD cache and start over
      the initialization from step 1.2. Then, the new DD tables will be used,
      and a normal restart will be done.
  3.3 Re-populate character sets and collations: The character set and
      collation information is read from files and added to a server
      internal data structure when the server starts. This data structure is,
      in turn, used to populate the corresponding DD tables. The tables must
      be re-populated on each server start if new character sets or collations
      have been added. However, we can not do this if in read only mode.
  3.4 Verify that the dictionary objects representing the core DD table meta
      data are present in the core registry of the storage adapter. If an
      object representing the meta data of a core DD table is not available,
      then we loose access to the DD tables, and we will not be able to handle
      cache misses or updates to the meta data.
  3.5 If an upgrade was done, the persistent version numbers are updated,
      e.g. the DD version and the current mysqld server version.

  @param thd            Thread context.

  @return       Upon failure, return true, otherwise false.
*/

bool restart(THD *thd);

/**
  Iterate through all the plugins, and store IS table meta data
  into dictionary, once during MySQL server bootstrap.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool store_plugin_IS_table_metadata(THD *thd);

/**
  Initialization and verification of dictionary objects
  after upgrade, similar to what is done after normal server
  restart.

  @param thd    Thread context
*/
bool setup_dd_objects_and_collations(THD *thd);

/**
  This function is used in case of crash during upgrade.
  It tries to initialize dictionary and calls DDSE_dict_recover.
  InnoDB should do the recovery and empty undo log. Upgrade
  process will do the cleanup and exit.

  @param thd    Thread context.
*/
void recover_innodb_upon_upgrade(THD *thd);

/**
  Initialize InnoDB for
  - creating new data directory : InnoDB creates system tablespace and
                                  dictionary tablespace.
  - normal server restart.      : Verifies existence of system and dictionary
                                  tablespaces.
  - in place upgrade            : Verifies existence of system tablespace and
                                  create dictionary tablespace.

  @param thd             Thread context.
  @param dict_init_mode  mode to initialize InnoDB
  @param version         Dictionary version.

  @return       Upon failure, return true, otherwise false.
*/
bool DDSE_dict_init(THD *thd, dict_init_mode_t dict_init_mode, uint version);

/**
  Create mysql schema. Create dictionary tables inside InnoDB.
  Create entry for dictionary tables inside dictionary tables.
  Add hard coded data to dictionary tables.
  Create Foreign key constraint on dictionary tables.

  This function is used in both cases, new data directory initialization
  and in place upgrade.

  @param thd            Thread context.
  @param is_dd_upgrade  Flag to indicate if it is in place upgrade.
  @param d              Dictionary instance

  @return       Upon failure, return true, otherwise false.

*/
bool initialize_dictionary(THD *thd, bool is_dd_upgrade, Dictionary_impl *d);

}  // namespace bootstrap

/**
  Helper function to do rollback or commit, depending on
  error. Also closes tables and releases transactional
  locks, regardless of error.

  @param thd   Thread
  @param error If true, the transaction will be rolledback.
               otherwise, it is committed.

  @returns false on success, otherwise true.
*/
bool end_transaction(THD *thd, bool error);

}  // namespace dd
#endif  // DD__BOOTSTRAPPER_INCLUDED
