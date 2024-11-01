#include "FussionPCH.h"

#include "Serialization/Serializer.h"

#include "Component.h"

namespace Fussion {

    void Component::Serialize(Serializer& ctx) const
    {
        ISerializable::Serialize(ctx);
    }

    void Component::Deserialize(Deserializer& ctx)
    {
        ISerializable::Deserialize(ctx);
    }
}
