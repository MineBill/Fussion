#include "ScriptingEngine.h"
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Math/Vector4.h>
#include <Fussion/Scene/Entity.h>
#include "ScriptBase.h"

#include "scriptstdstring/scriptstdstring.h"
#include "scriptarray/scriptarray.h"
#include "scriptany/scriptany.h"

namespace Fussion {

auto Vector2Construct(Vector2* self)
{
    new(self) Vector2();
};

auto Vector2CopyConstructor(Vector2 const& other, Vector2* self)
{
    new(self) Vector2(other);
};

auto Vector2InitConstructor(float x, float y, Vector2* self)
{
    new(self) Vector2(x, y);
};

auto Vector3Construct(Vector3* self)
{
    new(self) Vector3();
};

auto Vector3CopyConstructor(Vector3 const& other, Vector3* self)
{
    new(self) Vector3(other);
};

auto Vector3InitConstructor(float x, float y, float z, Vector3* self)
{
    new(self) Vector3(x, y, z);
};

auto Mat4Constructor(Mat4* self)
{
    new(self) Mat4();
}

auto Mat4ConstructorSingle(f32 x, Mat4* self)
{
    new(self) Mat4(x);
}

auto TransformConstructor(Transform* self)
{
    new(self) Transform();
}

void Print(std::string const& message)
{
    LOG_INFOF("[Script]: {}", message);
}

void LogDebug(std::string const& message, [[maybe_unused]] ScriptingEngine* engine)
{
    LOG_DEBUGF("[Script]: {}", message);
}

void LogWarn(std::string const& message, [[maybe_unused]] ScriptingEngine* engine)
{
    LOG_WARNF("[Script]: {}", message);
}

void LogError(std::string const& message, [[maybe_unused]] ScriptingEngine* engine)
{
    LOG_ERRORF("[Script]: {}", message);
}

f32 ASMax(f32 x, f32 y)
{
    return Math::Max(x, y);
}

f32 ASMin(f32 x, f32 y)
{
    return Math::Min(x, y);
}

void ScriptingEngine::RegisterTypes()
{
    RegisterStdString(m_ScriptEngine);
    RegisterScriptArray(m_ScriptEngine, true);
    RegisterScriptAny(m_ScriptEngine);

    m_ScriptEngine->RegisterGlobalFunction("void Print(const string &in message)", asFUNCTION(Print), asCALL_CDECL);

    {
        auto default_namespace = m_ScriptEngine->GetDefaultNamespace();
        defer(m_ScriptEngine->SetDefaultNamespace(default_namespace));

        auto r = m_ScriptEngine->SetDefaultNamespace("Log"); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("void Debug(const string &in message)", asFUNCTION(LogDebug), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("void Warn(const string &in message)", asFUNCTION(LogWarn), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("void Error(const string &in message)", asFUNCTION(LogError), asCALL_CDECL, this); VERIFY(r >= 0);
    }

    {
        auto default_namespace = m_ScriptEngine->GetDefaultNamespace();
        defer(m_ScriptEngine->SetDefaultNamespace(default_namespace));

        auto r = m_ScriptEngine->SetDefaultNamespace("Math"); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("float Abs(float)", asFUNCTION(Math::Abs<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("float Sin(float)", asFUNCTION(Math::Sin<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("float Cos(float)", asFUNCTION(Math::Cos<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("float Max(float, float)", asFUNCTION(ASMax), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("float Min(float, float)", asFUNCTION(ASMin), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("float Clamp(float value, float min, float max)", asFUNCTION(Math::Clamp<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("float Pow(float value, float power)", asFUNCTION(Math::Pow<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction("bool IsZero(float value)", asFUNCTION(Math::IsZero<f32>), asCALL_CDECL, this); VERIFY(r >= 0);
    }

    auto r = m_ScriptEngine->RegisterObjectType("Vector2", sizeof(Vector2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Vector2>()); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Vector2", "float X", offsetof(Vector2, X)); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Vector2", "float Y", offsetof(Vector2, Y)); VERIFY(r >= 0);

    r = m_ScriptEngine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector2Construct), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in)", asFUNCTION(Vector2CopyConstructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float, float y = 0)", asFUNCTION(Vector2InitConstructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);

    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 &opAddAssign(const Vector2 &in)", asMETHODPR(Vector2, operator+=, (const Vector2 &), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 &opAddAssign(float)", asMETHODPR(Vector2, operator+=, (float), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 &opMulAssign(float)", asMETHODPR(Vector2, operator*=, (float), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 &opDivAssign(float)", asMETHODPR(Vector2, operator/=, (float), Vector2&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "bool opEquals(const Vector2 &in) const", asFUNCTIONPR(operator==, (const Vector2&, const Vector2&), bool), asCALL_CDECL_OBJFIRST); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 opAdd(const Vector2 &in) const", asMETHODPR(Vector2, operator+, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 opSub(const Vector2 &in) const", asMETHODPR(Vector2, operator-, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 opMul(const Vector2 &in) const", asMETHODPR(Vector2, operator*, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 opMul(float) const", asMETHODPR(Vector2, operator*, (f32) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 opDiv(const Vector2 &in) const", asMETHODPR(Vector2, operator/, (const Vector2&) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "Vector2 opDiv(float) const", asMETHODPR(Vector2, operator/, (f32) const, Vector2), asCALL_THISCALL); VERIFY( r >= 0 );

    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "float Length() const", asMETHOD(Vector2, Length), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "float LengthSquared() const", asMETHOD(Vector2, LengthSquared), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "float DistanceTo(const Vector2 &in) const", asMETHOD(Vector2, DistanceTo), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "float DistanceToSquared(const Vector2 &in) const", asMETHOD(Vector2, DistanceToSquared), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "float Aspect() const", asMETHODPR(Vector2, Aspect, (void) const, f32), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector2", "bool IsZero() const", asMETHOD(Vector2, IsZero), asCALL_THISCALL); VERIFY( r >= 0 );

    r = m_ScriptEngine->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Vector3>()); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Vector3", "float X", asOFFSET(Vector3, X)); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Vector3", "float Y", asOFFSET(Vector3, Y)); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Vector3", "float Z", asOFFSET(Vector3, Z)); VERIFY(r >= 0);

    r = m_ScriptEngine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector3Construct), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in)", asFUNCTION(Vector3CopyConstructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float y = 0, float z = 0)", asFUNCTION(Vector3InitConstructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);

    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(const Vector3 &in)", asMETHODPR(Vector3, operator+=, (const Vector3 &), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(float)", asMETHODPR(Vector3, operator+=, (float), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 &opMulAssign(float)", asMETHODPR(Vector3, operator*=, (float), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 &opDivAssign(float)", asMETHODPR(Vector3, operator/=, (float), Vector3&), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "bool opEquals(const Vector3 &in) const", asFUNCTIONPR(operator==, (const Vector3&, const Vector3&), bool), asCALL_CDECL_OBJFIRST); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in) const", asMETHODPR(Vector3, operator+, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in) const", asMETHODPR(Vector3, operator-, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 opMul(const Vector3 &in) const", asMETHODPR(Vector3, operator*, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 opMul(float) const", asMETHODPR(Vector3, operator*, (f32) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 opDiv(const Vector3 &in) const", asMETHODPR(Vector3, operator/, (const Vector3&) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 opDiv(float) const", asMETHODPR(Vector3, operator/, (f32) const, Vector3), asCALL_THISCALL); VERIFY( r >= 0 );

    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "float Length() const", asMETHOD(Vector3, Length), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "float LengthSquared() const", asMETHOD(Vector3, LengthSquared), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "void Normalize()", asMETHOD(Vector3, Normalize), asCALL_THISCALL); VERIFY( r >= 0 );
    r = m_ScriptEngine->RegisterObjectMethod("Vector3", "Vector3 Normalized() const", asMETHOD(Vector3, Normalized), asCALL_THISCALL); VERIFY( r >= 0 );

    r = m_ScriptEngine->RegisterObjectType("Mat4", sizeof(Mat4), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Mat4>()); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("Mat4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Mat4Constructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("Mat4", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(Mat4ConstructorSingle), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
    // r = m_ScriptEngine->RegisterObjectMethod("Mat4", "", asFUNCTIONPR(glm::rotate));

    r = m_ScriptEngine->RegisterObjectType("Transform", sizeof(Transform), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Transform>()); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Transform", "Vector3 Position", asOFFSET(Transform, Position)); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Transform", "Vector3 EulerAngles", asOFFSET(Transform, EulerAngles)); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Transform", "Vector3 Scale", asOFFSET(Transform, Scale)); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("Transform", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(TransformConstructor), asCALL_CDECL_OBJLAST); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectMethod("Transform", "Vector3 get_Forward() const property", asMETHODPR(Transform, GetForward, () const, Vector3), asCALL_THISCALL); VERIFY(r >= 0);

    r = m_ScriptEngine->RegisterObjectType("Entity", sizeof(Entity), asOBJ_REF | asOBJ_NOCOUNT); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Entity", "string Name", asOFFSET(Entity, Name)); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectProperty("Entity", "Transform Transform", asOFFSET(Entity, Transform)); VERIFY(r >= 0);

    r = m_ScriptEngine->RegisterObjectType("ScriptBase", 0, asOBJ_REF); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("ScriptBase", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptBase, AddRef), asCALL_THISCALL); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("ScriptBase", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptBase, Release), asCALL_THISCALL); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectBehaviour("ScriptBase", asBEHAVE_FACTORY, "ScriptBase@ f(Entity@ owner)", asFUNCTION(ScriptBase::Create), asCALL_CDECL); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectMethod("ScriptBase", "ScriptBase &opAssign(const ScriptBase &in)", asMETHOD(ScriptBase, operator=), asCALL_THISCALL); VERIFY(r >= 0);

    r = m_ScriptEngine->RegisterObjectMethod("ScriptBase", "void OnStart()", asMETHOD(ScriptBase, OnStart), asCALL_THISCALL); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectMethod("ScriptBase", "void OnUpdate(float delta)", asMETHODPR(ScriptBase, OnUpdate, (f32), void), asCALL_THISCALL); VERIFY(r >= 0);
    r = m_ScriptEngine->RegisterObjectMethod("ScriptBase", "Entity@ GetOwner() const", asMETHODPR(ScriptBase, GetOwner, () const, Entity*), asCALL_THISCALL); VERIFY(r >= 0);
}

}