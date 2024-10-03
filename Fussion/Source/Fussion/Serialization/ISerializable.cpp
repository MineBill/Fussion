#include "FussionPCH.h"
#include "ISerializable.h"

namespace Fussion {
    void ISerializable::Serialize([[maybe_unused]] Serializer& ctx) const {}
    void ISerializable::Deserialize([[maybe_unused]] Deserializer& ctx) {}
}
