#include "FussionPCH.h"

#include "Serialization/Serializer.h"

#include "Component.h"

namespace Fussion {

    void Component::serialize(Serializer& ctx) const
    {
        ISerializable::serialize(ctx);
    }

    void Component::deserialize(Deserializer& ctx)
    {
        ISerializable::deserialize(ctx);
    }
}
