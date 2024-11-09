#include "Debug/Debug.h"
#include "Events/MouseEvents.h"
#include "Input/Keys.h"
#include "ScriptBase.h"
#include "ScriptingEngine.h"
#include <Fussion/Input/Input.h>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Math/Vector4.h>
#include <Fussion/Scene/Entity.h>

#include "Scene/Scene.h"
#include "aatc.hpp"
#include "aatc_container_mapped_templated_shared.hpp"
#include "aatc_container_unordered_map.hpp"
#include "aatc_container_vector.hpp"
#include "scriptany/scriptany.h"
#include "scriptarray/scriptarray.h"
#include "scriptstdstring/scriptstdstring.h"

#include <fmt/args.h>

namespace Fussion {

    namespace meta = meta_hpp;

    auto vector2_construct(Vector2* self)
    {
        new (self) Vector2();
    }

    auto vector2_copy_constructor(Vector2 const& other, Vector2* self)
    {
        new (self) Vector2(other);
    }

    auto vector2_init_constructor(float x, float y, Vector2* self)
    {
        new (self) Vector2(x, y);
    }

    auto vector3_construct(Vector3* self)
    {
        new (self) Vector3();
    }

    auto vector3_copy_constructor(Vector3 const& other, Vector3* self)
    {
        new (self) Vector3(other);
    }

    auto vector3_init_constructor(float x, float y, float z, Vector3* self)
    {
        new (self) Vector3(x, y, z);
    }

    auto color_construct(Color* self)
    {
        new (self) Color();
    }

    auto color_copy_constructor(Color const& other, Color* self)
    {
        new (self) Color(other);
    }

    auto color_init_constructor(float r, float g, float b, float a, Vector3* self)
    {
        new (self) Color(r, g, b, a);
    }

    auto mat4_constructor(Mat4* self)
    {
        new (self) Mat4();
    }

    auto mat4_constructor_single(f32 x, Mat4* self)
    {
        new (self) Mat4(x);
    }

    auto transform_constructor(Transform* self)
    {
        new (self) Transform();
    }

    CScriptArray* Entity_GetChildren(Entity* entity)
    {
        auto ctx = asGetActiveContext();
        auto ti = ctx->GetEngine()->GetTypeInfoByDecl("Array<Entity@>");

        auto const& children = entity->children();
        auto arr = CScriptArray::Create(ti, CAST(asUINT, children.size()));
        for (size_t i = 0; i < children.size(); ++i) {
            Entity* e = entity->scene().get_entity(children[i]);
            arr->SetValue(CAST(asUINT, i), CAST(void*, &e));
        }
        return arr;
    }

    void Print(std::string const& message, CScriptArray* array)
    {
        fmt::dynamic_format_arg_store<fmt::format_context> args;
        for (u32 i = 0; i < array->GetSize(); ++i) {
            auto* any = CAST(CScriptAny*, array->At(i));
            auto typeId = any->GetTypeId();
            switch (typeId) {
            case asTYPEID_INT8:
            case asTYPEID_INT16:
            case asTYPEID_INT32:
            case asTYPEID_INT64:
                s64 value;
                any->Retrieve(value);
                args.push_back(value);
                break;
            case asTYPEID_FLOAT:
            case asTYPEID_DOUBLE:
                f64 fvalue;
                any->Retrieve(fvalue);
                args.push_back(fvalue);
                break;
            case asTYPEID_BOOL:
                bool bvalue;
                any->Retrieve(&bvalue, asTYPEID_BOOL);
                args.push_back(bvalue);
                break;
            default:
                if (typeId & asTYPEID_SCRIPTOBJECT) {
                    auto ctx = asGetActiveContext();
                    if (auto info = ctx->GetEngine()->GetTypeInfoById(typeId)) {
                        if (auto function = info->GetMethodByDecl("string ToString() const")) {
                            auto local_ctx = ctx->GetEngine()->CreateContext();
                            defer(local_ctx->Release());
                            local_ctx->Prepare(function);

                            local_ctx->SetObject(any->GetAsPtr());
                            auto status = local_ctx->Execute();
                            if (status == asEXECUTION_FINISHED) {
                                auto ret = CAST(std::string*, local_ctx->GetAddressOfReturnValue());
                                if (ret) {
                                    args.push_back(*ret);
                                }
                            } else {
                                LOG_ERRORF("{}", local_ctx->GetExceptionString());
                            }
                        } else {
                            args.push_back(info->GetName());
                        }
                    }
                    args.push_back("(script object)");
                } else if (typeId & asTYPEID_APPOBJECT) {
                    args.push_back("(app object)");
                } else {
                    args.push_back("(unknown type)");
                }
            }
        }
        Log::default_logger()->write(LogLevel::Debug, fmt::vformat(message, args));
    }

    void LogDebug(std::string const& message)
    {
        LOG_DEBUGF("[Script]: {}", message);
    }

    void LogWarn(std::string const& message)
    {
        LOG_WARNF("[Script]: {}", message);
    }

    void LogError(std::string const& message)
    {
        LOG_ERRORF("[Script]: {}", message);
    }

    f32 As_Max(f32 x, f32 y)
    {
        return Math::max(x, y);
    }

    f32 As_Min(f32 x, f32 y)
    {
        return Math::min(x, y);
    }

    auto GetDict() -> aatc::container::mapped::templated::unordered_map*
    {
        asITypeInfo* tinfo = asGetActiveContext()->GetEngine()->GetTypeInfoByDecl("Dictionary<string,int>");
        auto result = new aatc::container::mapped::templated::unordered_map(tinfo);

        std::string pepegas = "pepegas";
        int value = 10;
        result->insert(&pepegas, &value);
        return result;
    }

    void ScriptingEngine::register_types()
    {
        // clang-format off
        RegisterStdString(m_script_engine);
        RegisterScriptArray(m_script_engine, true);
        RegisterScriptAny(m_script_engine);

        aatc::RegisterAllContainers(m_script_engine);

        auto r = m_script_engine->RegisterGlobalFunction("void Print(const string &in fmt, const Array<Any>&in args = {})", asFUNCTION(Print), asCALL_CDECL); VERIFY(r >= 0);

        r = m_script_engine->RegisterGlobalFunction("Dictionary<string, int>@ GetDict()", asFUNCTION(GetDict), asCALL_CDECL); VERIFY(r >= 0);
        {
            auto default_namespace = m_script_engine->GetDefaultNamespace();
            defer(m_script_engine->SetDefaultNamespace(default_namespace));

            r = m_script_engine->SetDefaultNamespace("Log"); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("void debug(const string &in message)", asFUNCTION(LogDebug), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("void warn(const string &in message)", asFUNCTION(LogWarn), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("void error(const string &in message)", asFUNCTION(LogError), asCALL_CDECL, this); VERIFY(r >= 0);
        }

        {
            auto default_namespace = m_script_engine->GetDefaultNamespace();
            defer(m_script_engine->SetDefaultNamespace(default_namespace));

            r = m_script_engine->SetDefaultNamespace("Math"); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("float Abs(float)", asFUNCTION(Math::abs<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("float Sin(float)", asFUNCTION(Math::sin<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("float Cos(float)", asFUNCTION(Math::cos<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("float Max(float, float)", asFUNCTION(As_Max), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("float Min(float, float)", asFUNCTION(As_Min), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("float Clamp(float value, float min, float max)", asFUNCTION(Math::clamp<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("float Pow(float value, float power)", asFUNCTION(Math::pow<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("bool IsZero(float value)", asFUNCTION(Math::is_zero<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
        }

        r = m_script_engine->RegisterObjectType("Vector2", sizeof(Vector2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Vector2>()); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Vector2", "float x", offsetof(Vector2, x)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Vector2", "float y", offsetof(Vector2, y)); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(vector2_construct), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in)", asFUNCTION(vector2_copy_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float, float y = 0)", asFUNCTION(vector2_init_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 &opAssign(const Vector2 &in)", asMETHODPR(Vector2, operator=, (const Vector2 &), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 &opAddAssign(const Vector2 &in)", asMETHODPR(Vector2, operator+=, (const Vector2 &), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 &opAddAssign(float)", asMETHODPR(Vector2, operator+=, (float), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 &opMulAssign(float)", asMETHODPR(Vector2, operator*=, (float), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 &opDivAssign(float)", asMETHODPR(Vector2, operator/=, (float), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "bool opEquals(const Vector2 &in) const", asFUNCTIONPR(operator==, (const Vector2&, const Vector2&), bool), asCALL_CDECL_OBJFIRST); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 opAdd(const Vector2 &in) const", asMETHODPR(Vector2, operator+, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 opSub(const Vector2 &in) const", asMETHODPR(Vector2, operator-, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 opMul(const Vector2 &in) const", asMETHODPR(Vector2, operator*, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 opMul(float) const", asMETHODPR(Vector2, operator*, (f32) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 opDiv(const Vector2 &in) const", asMETHODPR(Vector2, operator/, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "Vector2 opDiv(float) const", asMETHODPR(Vector2, operator/, (f32) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );

        r = m_script_engine->RegisterObjectMethod("Vector2", "float Length() const", asMETHOD(Vector2, length), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "float LengthSquared() const", asMETHOD(Vector2, length_squared), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "float DistanceTo(const Vector2 &in) const", asMETHOD(Vector2, distance_to), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "float DistanceToSquared(const Vector2 &in) const", asMETHOD(Vector2, distance_to_squared), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "float Aspect() const", asMETHODPR(Vector2, aspect, () const, f32), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector2", "bool IsZero() const", asMETHOD(Vector2, is_zero), asCALL_THISCALL); VERIFY( r >= 0 );

        r = m_script_engine->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS | asGetTypeTraits<Vector3>()); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Vector3", "float x", asOFFSET(Vector3, x)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Vector3", "float y", asOFFSET(Vector3, y)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Vector3", "float z", asOFFSET(Vector3, z)); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(vector3_construct), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in)", asFUNCTION(vector3_copy_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float y = 0, float z = 0)", asFUNCTION(vector3_init_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 &opAssign(const Vector3 &in)", asMETHODPR(Vector3, operator=, (const Vector3 &), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(const Vector3 &in)", asMETHODPR(Vector3, operator+=, (const Vector3 &), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(float)", asMETHODPR(Vector3, operator+=, (float), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 &opMulAssign(float)", asMETHODPR(Vector3, operator*=, (float), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 &opDivAssign(float)", asMETHODPR(Vector3, operator/=, (float), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "bool opEquals(const Vector3 &in) const", asFUNCTIONPR(operator==, (const Vector3&, const Vector3&), bool), asCALL_CDECL_OBJFIRST); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in) const", asMETHODPR(Vector3, operator+, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in) const", asMETHODPR(Vector3, operator-, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 opMul(const Vector3 &in) const", asMETHODPR(Vector3, operator*, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 opMul(float) const", asMETHODPR(Vector3, operator*, (f32) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 opDiv(const Vector3 &in) const", asMETHODPR(Vector3, operator/, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 opDiv(float) const", asMETHODPR(Vector3, operator/, (f32) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );

        r = m_script_engine->RegisterObjectMethod("Vector3", "float Length() const", asMETHOD(Vector3, length), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "float LengthSquared() const", asMETHOD(Vector3, length_squared), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "void Normalize()", asMETHOD(Vector3, normalize), asCALL_THISCALL); VERIFY( r >= 0 );
        r = m_script_engine->RegisterObjectMethod("Vector3", "Vector3 Normalized() const", asMETHOD(Vector3, normalized), asCALL_THISCALL); VERIFY( r >= 0 );

        r = m_script_engine->RegisterObjectType("Color", sizeof(Color), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Color>()); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Color", "float r", asOFFSET(Color, r)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Color", "float g", asOFFSET(Color, g)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Color", "float b", asOFFSET(Color, b)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Color", "float a", asOFFSET(Color, a)); VERIFY(r >= 0);

        {
            auto keys_type = meta_hpp::resolve_type<Keys>(); VERIFY(keys_type.is_valid());
            auto& metadata = keys_type.get_metadata();
            auto name = metadata.at("Name").as<std::string>();
            r = m_script_engine->RegisterEnum(name.data()); VERIFY(r >= 0);

            for (auto const& evalue : keys_type.get_evalues()) {
                r = m_script_engine->RegisterEnumValue(name.data(), evalue.get_name().data(), evalue.get_underlying_value().as<int>()); VERIFY(r >= 0);
            }
        }

        {
            auto keys_type = meta::resolve_type<MouseButton>(); VERIFY(keys_type.is_valid());
            auto& metadata = keys_type.get_metadata();
            auto name = metadata.at("Name").as<std::string>();
            r = m_script_engine->RegisterEnum(name.data()); VERIFY(r >= 0);

            for (auto const& evalue : keys_type.get_evalues()) {
                r = m_script_engine->RegisterEnumValue(name.data(), evalue.get_name().data(), evalue.get_underlying_value().as<int>()); VERIFY(r >= 0);
            }
        }

        {
            auto default_namespace = m_script_engine->GetDefaultNamespace();
            defer(m_script_engine->SetDefaultNamespace(default_namespace));

            auto input = meta::resolve_type<Input>();
            auto& metadata = input.get_metadata();
            auto& name = metadata.at("Name").as<std::string>();

            r = m_script_engine->SetDefaultNamespace(name.data()); VERIFY(r >= 0);

            r = m_script_engine->RegisterGlobalFunction("bool IsKeyPressed(Keys key)", asFUNCTION(Input::is_key_pressed), asCALL_CDECL); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("bool IsKeyReleased(Keys key)", asFUNCTION(Input::is_key_released), asCALL_CDECL); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("bool IsKeyDown(Keys key)", asFUNCTION(Input::is_key_down), asCALL_CDECL); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("bool IsKeyUp(Keys key)", asFUNCTION(Input::is_key_up), asCALL_CDECL); VERIFY(r >= 0);

            auto isAnyKeyDown = [](CScriptArray* keys) -> bool {
                for (u32 i = 0; i < keys->GetSize(); i++) {
                    if (auto key = CAST(Keys*, keys->At(i)); Input::is_key_down(*key)) {
                        return true;
                    }
                }
                return false;
            };
            r = m_script_engine->RegisterGlobalFunction("bool IsAnyKeyDown(const Keys[] &in key)", asFUNCTION(CAST(bool(*)(CScriptArray*), isAnyKeyDown)), asCALL_CDECL); VERIFY(r >= 0);
        }

        {
            auto default_namespace = m_script_engine->GetDefaultNamespace();
            defer(m_script_engine->SetDefaultNamespace(default_namespace));

            auto k = m_script_engine->SetDefaultNamespace("Color"); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color White", const_cast<Color*>(&Color::White)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Red", const_cast<Color*>(&Color::Red)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Green", const_cast<Color*>(&Color::Green)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Blue", const_cast<Color*>(&Color::Blue)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Yellow", const_cast<Color*>(&Color::Yellow)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Magenta", const_cast<Color*>(&Color::Magenta)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Cyan", const_cast<Color*>(&Color::Cyan)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Black", const_cast<Color*>(&Color::Black)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Purple", const_cast<Color*>(&Color::Purple)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Turquoise", const_cast<Color*>(&Color::Turquoise)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Lime", const_cast<Color*>(&Color::Lime)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Gray", const_cast<Color*>(&Color::Gray)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Brown", const_cast<Color*>(&Color::Brown)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Maroon", const_cast<Color*>(&Color::Maroon)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Teal", const_cast<Color*>(&Color::Teal)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Olive", const_cast<Color*>(&Color::Olive)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Navy", const_cast<Color*>(&Color::Navy)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Coral", const_cast<Color*>(&Color::Coral)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Rose", const_cast<Color*>(&Color::Rose)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color SkyBlue", const_cast<Color*>(&Color::SkyBlue)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color ForestGreen", const_cast<Color*>(&Color::ForestGreen)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color DarkGoldenRod", const_cast<Color*>(&Color::DarkGoldenRod)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Indigo", const_cast<Color*>(&Color::Indigo)); VERIFY(k >= 0);
            k = m_script_engine->RegisterGlobalProperty("Color Transparent", const_cast<Color*>(&Color::Transparent)); VERIFY(k >= 0);
        }

        r = m_script_engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(color_construct), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(const Color &in)", asFUNCTION(color_copy_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float r, float g = 0, float b = 0, float a = 0)", asFUNCTION(color_init_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectType("Mat4", sizeof(Mat4), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Mat4>()); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Mat4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(mat4_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Mat4", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(mat4_constructor_single), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectType("Transform", sizeof(Transform), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Transform>()); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Transform", "Vector3 Position", asOFFSET(Transform, Position)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Transform", "Vector3 EulerAngles", asOFFSET(Transform, EulerAngles)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Transform", "Vector3 Scale", asOFFSET(Transform, Scale)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("Transform", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(transform_constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectMethod("Transform", "Vector3 get_Forward() const property", asMETHODPR(Transform, forward, () const, Vector3), asCALL_THISCALL); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectType("Entity", sizeof(Entity), asOBJ_REF | asOBJ_NOCOUNT); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Entity", "string Name", asOFFSET(Entity, name)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectProperty("Entity", "Transform WorldTransform", asOFFSET(Entity, transform)); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectMethod("Entity", "Array<Entity@>@ GetChildren() const", asFUNCTION(Entity_GetChildren), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectType("ScriptBase", 0, asOBJ_REF); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("ScriptBase", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptBase, AddRef), asCALL_THISCALL); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("ScriptBase", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptBase, Release), asCALL_THISCALL); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectBehaviour("ScriptBase", asBEHAVE_FACTORY, "ScriptBase@ f(Entity@ owner)", asFUNCTION(ScriptBase::Create), asCALL_CDECL); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectMethod("ScriptBase", "ScriptBase &opAssign(const ScriptBase &in)", asMETHOD(ScriptBase, operator=), asCALL_THISCALL); VERIFY(r >= 0);

        r = m_script_engine->RegisterObjectMethod("ScriptBase", "void OnStart()", asMETHOD(ScriptBase, OnStart), asCALL_THISCALL); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectMethod("ScriptBase", "void OnUpdate(float delta)", asMETHODPR(ScriptBase, OnUpdate, (f32), void), asCALL_THISCALL); VERIFY(r >= 0);
        r = m_script_engine->RegisterObjectMethod("ScriptBase", "Entity@ GetOwner() const", asMETHODPR(ScriptBase, GetOwner, () const, Entity*), asCALL_THISCALL); VERIFY(r >= 0);

        {
            auto default_namespace = m_script_engine->GetDefaultNamespace();
            defer(m_script_engine->SetDefaultNamespace(default_namespace));

            r = m_script_engine->SetDefaultNamespace("Debug"); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("void DrawLine(Vector3 start, Vector3 end, float time = 0.0f, Color color = Color::Red)", asFUNCTION(Debug::draw_line), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("void DrawCube(Vector3 center, Vector3 euler_angles, Vector3 size, float time = 0.0f, Color color = Color::Red)", asFUNCTION(static_cast<void(*)(Vector3, Vector3, f32, Color)>(Debug::draw_cube)), asCALL_CDECL, this); VERIFY(r >= 0);
            r = m_script_engine->RegisterGlobalFunction("void DrawSphere(Vector3 center, Vector3 euler_angles, float radius, float time = 0.0f, Color color = Color::Red)", asFUNCTION(Debug::draw_sphere), asCALL_CDECL, this); VERIFY(r >= 0);
        }
        // clang-format on
    }
}
