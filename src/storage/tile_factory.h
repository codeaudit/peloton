/*-------------------------------------------------------------------------
 *
 * tile_factory.h
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /n-store/src/storage/tile_factory.h
 *
 *-------------------------------------------------------------------------
 */

#pragma once

#include "catalog/schema.h"
#include "storage/tile.h"
#include "storage/physical_tile.h"
#include "storage/backend.h"
#include "storage/volatile_backend.h"

namespace nstore {
namespace storage {

//===--------------------------------------------------------------------===//
// Tile factory
//===--------------------------------------------------------------------===//

class TileFactory {
public:
	TileFactory();
	virtual ~TileFactory();

	static PhysicalTile *GetPhysicalTile(catalog::Schema* schema,
			int tuple_count, const std::vector<std::string>& column_names,
			const bool owns_tuple_schema, int tile_size);

	static PhysicalTile *GetPhysicalTile(Oid database_id, Oid table_id, Oid tile_group_id,
			Backend* backend, catalog::Schema* schema, int tuple_count,
			const std::vector<std::string>& column_names,
			const bool owns_tuple_schema, int tile_size);

private:

	static void InitCommon(PhysicalTile *tile, Oid database_id, Oid table_id,
			Oid tile_group_id, catalog::Schema* schema,
			const std::vector<std::string>& column_names,
			const bool owns_tuple_schema, int tile_size);

};

} // End storage namespace
} // End nstore namespace


