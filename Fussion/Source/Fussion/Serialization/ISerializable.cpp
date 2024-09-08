#include "FussionPCH.h"
#include "ISerializable.h"

namespace Fussion {
    void ISerializable::serialize([[maybe_unused]] Serializer& ctx) const {}
    void ISerializable::deserialize([[maybe_unused]] Deserializer& ctx) {}
}
