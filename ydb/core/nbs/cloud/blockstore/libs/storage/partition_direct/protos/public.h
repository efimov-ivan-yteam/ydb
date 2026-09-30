#pragma once

#include <util/generic/map.h>
#include <util/system/types.h>

namespace NYdb::NBS::PartitionDirect::NProto {

////////////////////////////////////////////////////////////////////////////////

class TBlockField;
class TDDiskState;
class TDirtyMapState;
class TDirectBlockGroupHealth;

////////////////////////////////////////////////////////////////////////////////

}   // namespace NYdb::NBS::PartitionDirect::NProto

namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect {

////////////////////////////////////////////////////////////////////////////////

using TBlockFieldProto = NYdb::NBS::PartitionDirect::NProto::TBlockField;
using TDDiskStateProto = NYdb::NBS::PartitionDirect::NProto::TDDiskState;
using TDirtyMapStateProto = NYdb::NBS::PartitionDirect::NProto::TDirtyMapState;
using TDirectBlockGroupHealthProto = NYdb::NBS::PartitionDirect::NProto::TDirectBlockGroupHealth;

using TDirtyMapStateProtos = TMap<ui32, TDirtyMapStateProto>;
using TDirectBlockGroupHealthProtos = TMap<ui32, TDirectBlockGroupHealthProto>;

////////////////////////////////////////////////////////////////////////////////

}   // namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect
