// Autogenerated by gameplay-luagen
#include "Base.h"
#include "ScriptController.h"
#include "lua_PhysicsGhostObject.h"
#include "Animation.h"
#include "AnimationTarget.h"
#include "Base.h"
#include "Game.h"
#include "MaterialParameter.h"
#include "Node.h"
#include "PhysicsCharacter.h"
#include "PhysicsCollisionObject.h"
#include "PhysicsController.h"
#include "PhysicsGhostObject.h"
#include "PhysicsRigidBody.h"
#include "PhysicsVehicle.h"
#include "PhysicsVehicleWheel.h"
#include "ScriptController.h"
#include "ScriptTarget.h"
#include "Transform.h"
#include "PhysicsCharacter.h"
#include "PhysicsCollisionObject.h"
#include "Transform.h"

namespace gameplay
{

extern void luaGlobal_Register_Conversion_Function(const char* className, void*(*func)(void*, const char*));

static PhysicsGhostObject* getInstance(lua_State* state)
{
    void* userdata = luaL_checkudata(state, 1, "PhysicsGhostObject");
    luaL_argcheck(state, userdata != NULL, 1, "'PhysicsGhostObject' expected.");
    return (PhysicsGhostObject*)((gameplay::ScriptUtil::LuaObject*)userdata)->instance;
}

static int lua_PhysicsGhostObject_addCollisionListener(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 2:
        {
            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TUSERDATA || lua_type(state, 2) == LUA_TTABLE || lua_type(state, 2) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    bool param1Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject::CollisionListener> param1 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject::CollisionListener>(2, "PhysicsCollisionObjectCollisionListener", false, &param1Valid);
                    if (!param1Valid)
                        break;

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->addCollisionListener(param1);
                    
                    return 0;
                }
            } while (0);

            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TSTRING || lua_type(state, 2) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    const char* param1 = gameplay::ScriptUtil::getString(2, false);

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->addCollisionListener(param1);
                    
                    return 0;
                }
            } while (0);

            lua_pushstring(state, "lua_PhysicsGhostObject_addCollisionListener - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        case 3:
        {
            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TUSERDATA || lua_type(state, 2) == LUA_TTABLE || lua_type(state, 2) == LUA_TNIL) &&
                    (lua_type(state, 3) == LUA_TUSERDATA || lua_type(state, 3) == LUA_TTABLE || lua_type(state, 3) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    bool param1Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject::CollisionListener> param1 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject::CollisionListener>(2, "PhysicsCollisionObjectCollisionListener", false, &param1Valid);
                    if (!param1Valid)
                        break;

                    // Get parameter 2 off the stack.
                    bool param2Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject> param2 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject>(3, "PhysicsCollisionObject", false, &param2Valid);
                    if (!param2Valid)
                        break;

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->addCollisionListener(param1, param2);
                    
                    return 0;
                }
            } while (0);

            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TSTRING || lua_type(state, 2) == LUA_TNIL) &&
                    (lua_type(state, 3) == LUA_TUSERDATA || lua_type(state, 3) == LUA_TTABLE || lua_type(state, 3) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    const char* param1 = gameplay::ScriptUtil::getString(2, false);

                    // Get parameter 2 off the stack.
                    bool param2Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject> param2 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject>(3, "PhysicsCollisionObject", false, &param2Valid);
                    if (!param2Valid)
                        break;

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->addCollisionListener(param1, param2);
                    
                    return 0;
                }
            } while (0);

            lua_pushstring(state, "lua_PhysicsGhostObject_addCollisionListener - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 2 or 3).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_collidesWith(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 2:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                (lua_type(state, 2) == LUA_TUSERDATA || lua_type(state, 2) == LUA_TTABLE || lua_type(state, 2) == LUA_TNIL))
            {
                // Get parameter 1 off the stack.
                bool param1Valid;
                gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject> param1 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject>(2, "PhysicsCollisionObject", false, &param1Valid);
                if (!param1Valid)
                {
                    lua_pushstring(state, "Failed to convert parameter 1 to type 'PhysicsCollisionObject'.");
                    lua_error(state);
                }

                PhysicsGhostObject* instance = getInstance(state);
                bool result = instance->collidesWith(param1);

                // Push the return value onto the stack.
                lua_pushboolean(state, result);

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_collidesWith - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 2).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_getCollisionShape(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                void* returnPtr = ((void*)instance->getCollisionShape());
                if (returnPtr)
                {
                    gameplay::ScriptUtil::LuaObject* object = (gameplay::ScriptUtil::LuaObject*)lua_newuserdata(state, sizeof(gameplay::ScriptUtil::LuaObject));
                    object->instance = returnPtr;
                    object->owns = false;
                    luaL_getmetatable(state, "PhysicsCollisionShape");
                    lua_setmetatable(state, -2);
                }
                else
                {
                    lua_pushnil(state);
                }

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_getCollisionShape - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_getNode(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                void* returnPtr = ((void*)instance->getNode());
                if (returnPtr)
                {
                    gameplay::ScriptUtil::LuaObject* object = (gameplay::ScriptUtil::LuaObject*)lua_newuserdata(state, sizeof(gameplay::ScriptUtil::LuaObject));
                    object->instance = returnPtr;
                    object->owns = false;
                    luaL_getmetatable(state, "Node");
                    lua_setmetatable(state, -2);
                }
                else
                {
                    lua_pushnil(state);
                }

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_getNode - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_getShapeType(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                PhysicsCollisionShape::Type result = instance->getShapeType();

                // Push the return value onto the stack.
                lua_pushnumber(state, (int)result);

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_getShapeType - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_getType(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                PhysicsCollisionObject::Type result = instance->getType();

                // Push the return value onto the stack.
                lua_pushnumber(state, (int)result);

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_getType - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_isDynamic(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                bool result = instance->isDynamic();

                // Push the return value onto the stack.
                lua_pushboolean(state, result);

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_isDynamic - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_isEnabled(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                bool result = instance->isEnabled();

                // Push the return value onto the stack.
                lua_pushboolean(state, result);

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_isEnabled - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_isKinematic(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                bool result = instance->isKinematic();

                // Push the return value onto the stack.
                lua_pushboolean(state, result);

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_isKinematic - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_isStatic(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 1:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA))
            {
                PhysicsGhostObject* instance = getInstance(state);
                bool result = instance->isStatic();

                // Push the return value onto the stack.
                lua_pushboolean(state, result);

                return 1;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_isStatic - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 1).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_removeCollisionListener(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 2:
        {
            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TUSERDATA || lua_type(state, 2) == LUA_TTABLE || lua_type(state, 2) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    bool param1Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject::CollisionListener> param1 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject::CollisionListener>(2, "PhysicsCollisionObjectCollisionListener", false, &param1Valid);
                    if (!param1Valid)
                        break;

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->removeCollisionListener(param1);
                    
                    return 0;
                }
            } while (0);

            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TSTRING || lua_type(state, 2) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    const char* param1 = gameplay::ScriptUtil::getString(2, false);

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->removeCollisionListener(param1);
                    
                    return 0;
                }
            } while (0);

            lua_pushstring(state, "lua_PhysicsGhostObject_removeCollisionListener - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        case 3:
        {
            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TUSERDATA || lua_type(state, 2) == LUA_TTABLE || lua_type(state, 2) == LUA_TNIL) &&
                    (lua_type(state, 3) == LUA_TUSERDATA || lua_type(state, 3) == LUA_TTABLE || lua_type(state, 3) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    bool param1Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject::CollisionListener> param1 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject::CollisionListener>(2, "PhysicsCollisionObjectCollisionListener", false, &param1Valid);
                    if (!param1Valid)
                        break;

                    // Get parameter 2 off the stack.
                    bool param2Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject> param2 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject>(3, "PhysicsCollisionObject", false, &param2Valid);
                    if (!param2Valid)
                        break;

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->removeCollisionListener(param1, param2);
                    
                    return 0;
                }
            } while (0);

            do
            {
                if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                    (lua_type(state, 2) == LUA_TSTRING || lua_type(state, 2) == LUA_TNIL) &&
                    (lua_type(state, 3) == LUA_TUSERDATA || lua_type(state, 3) == LUA_TTABLE || lua_type(state, 3) == LUA_TNIL))
                {
                    // Get parameter 1 off the stack.
                    const char* param1 = gameplay::ScriptUtil::getString(2, false);

                    // Get parameter 2 off the stack.
                    bool param2Valid;
                    gameplay::ScriptUtil::LuaArray<PhysicsCollisionObject> param2 = gameplay::ScriptUtil::getObjectPointer<PhysicsCollisionObject>(3, "PhysicsCollisionObject", false, &param2Valid);
                    if (!param2Valid)
                        break;

                    PhysicsGhostObject* instance = getInstance(state);
                    instance->removeCollisionListener(param1, param2);
                    
                    return 0;
                }
            } while (0);

            lua_pushstring(state, "lua_PhysicsGhostObject_removeCollisionListener - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 2 or 3).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_setEnabled(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 2:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                lua_type(state, 2) == LUA_TBOOLEAN)
            {
                // Get parameter 1 off the stack.
                bool param1 = gameplay::ScriptUtil::luaCheckBool(state, 2);

                PhysicsGhostObject* instance = getInstance(state);
                instance->setEnabled(param1);
                
                return 0;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_setEnabled - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 2).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

static int lua_PhysicsGhostObject_transformChanged(lua_State* state)
{
    // Get the number of parameters.
    int paramCount = lua_gettop(state);

    // Attempt to match the parameters to a valid binding.
    switch (paramCount)
    {
        case 3:
        {
            if ((lua_type(state, 1) == LUA_TUSERDATA) &&
                (lua_type(state, 2) == LUA_TUSERDATA || lua_type(state, 2) == LUA_TTABLE || lua_type(state, 2) == LUA_TNIL) &&
                lua_type(state, 3) == LUA_TNUMBER)
            {
                // Get parameter 1 off the stack.
                bool param1Valid;
                gameplay::ScriptUtil::LuaArray<Transform> param1 = gameplay::ScriptUtil::getObjectPointer<Transform>(2, "Transform", false, &param1Valid);
                if (!param1Valid)
                {
                    lua_pushstring(state, "Failed to convert parameter 1 to type 'Transform'.");
                    lua_error(state);
                }

                // Get parameter 2 off the stack.
                long param2 = (long)luaL_checklong(state, 3);

                PhysicsGhostObject* instance = getInstance(state);
                instance->transformChanged(param1, param2);
                
                return 0;
            }

            lua_pushstring(state, "lua_PhysicsGhostObject_transformChanged - Failed to match the given parameters to a valid function signature.");
            lua_error(state);
            break;
        }
        default:
        {
            lua_pushstring(state, "Invalid number of parameters (expected 3).");
            lua_error(state);
            break;
        }
    }
    return 0;
}

// Provides support for conversion to all known relative types of PhysicsGhostObject
static void* __convertTo(void* ptr, const char* typeName)
{
    PhysicsGhostObject* ptrObject = reinterpret_cast<PhysicsGhostObject*>(ptr);

    if (strcmp(typeName, "PhysicsCharacter") == 0)
    {
        return reinterpret_cast<void*>(static_cast<PhysicsCharacter*>(ptrObject));
    }
    else if (strcmp(typeName, "PhysicsCollisionObject") == 0)
    {
        return reinterpret_cast<void*>(static_cast<PhysicsCollisionObject*>(ptrObject));
    }
    else if (strcmp(typeName, "Transform::Listener") == 0)
    {
        return reinterpret_cast<void*>(static_cast<Transform::Listener*>(ptrObject));
    }

    // No conversion available for 'typeName'
    return NULL;
}

static int lua_PhysicsGhostObject_to(lua_State* state)
{
    // There should be only a single parameter (this instance)
    if (lua_gettop(state) != 2 || lua_type(state, 1) != LUA_TUSERDATA || lua_type(state, 2) != LUA_TSTRING)
    {
        lua_pushstring(state, "lua_PhysicsGhostObject_to - Invalid number of parameters (expected 2).");
        lua_error(state);
        return 0;
    }

    PhysicsGhostObject* instance = getInstance(state);
    const char* typeName = gameplay::ScriptUtil::getString(2, false);
    void* result = __convertTo((void*)instance, typeName);

    if (result)
    {
        gameplay::ScriptUtil::LuaObject* object = (gameplay::ScriptUtil::LuaObject*)lua_newuserdata(state, sizeof(gameplay::ScriptUtil::LuaObject));
        object->instance = (void*)result;
        object->owns = false;
        luaL_getmetatable(state, typeName);
        lua_setmetatable(state, -2);
    }
    else
    {
        lua_pushnil(state);
    }

    return 1;
}

void luaRegister_PhysicsGhostObject()
{
    const luaL_Reg lua_members[] = 
    {
        {"addCollisionListener", lua_PhysicsGhostObject_addCollisionListener},
        {"collidesWith", lua_PhysicsGhostObject_collidesWith},
        {"getCollisionShape", lua_PhysicsGhostObject_getCollisionShape},
        {"getNode", lua_PhysicsGhostObject_getNode},
        {"getShapeType", lua_PhysicsGhostObject_getShapeType},
        {"getType", lua_PhysicsGhostObject_getType},
        {"isDynamic", lua_PhysicsGhostObject_isDynamic},
        {"isEnabled", lua_PhysicsGhostObject_isEnabled},
        {"isKinematic", lua_PhysicsGhostObject_isKinematic},
        {"isStatic", lua_PhysicsGhostObject_isStatic},
        {"removeCollisionListener", lua_PhysicsGhostObject_removeCollisionListener},
        {"setEnabled", lua_PhysicsGhostObject_setEnabled},
        {"transformChanged", lua_PhysicsGhostObject_transformChanged},
        {"to", lua_PhysicsGhostObject_to},
        {NULL, NULL}
    };
    const luaL_Reg* lua_statics = NULL;
    std::vector<std::string> scopePath;

    gameplay::ScriptUtil::registerClass("PhysicsGhostObject", lua_members, NULL, NULL, lua_statics, scopePath);

    luaGlobal_Register_Conversion_Function("PhysicsGhostObject", __convertTo);
}

}
