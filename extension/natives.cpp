/**
 * vim: set ts=4 :
 * =============================================================================
 * SendVar Proxy Manager
 * Copyright (C) 2011-2019 Afronanny & AlliedModders community.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "natives.h"
#include "wrappers.h"

static cell_t Native_Hook(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * propName;
	pContext->LocalToString(params[2], &propName);
	CHECK_VALID_ENTITY_SENDPROP(entity, propName)

	SendProp *pProp = info.prop;
	PropType propType = static_cast<PropType>(params[3]);
	CHECK_PROP_BASE_DATA_TYPE(pProp, propType)
	
	SendPropHook hook;
	hook.objectID = entity;
	hook.sCallbackInfo.pCallback = (void *)pContext->GetFunctionById(params[4]);
	hook.sCallbackInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	hook.sCallbackInfo.pOwner = (void *)pContext;
	hook.pEnt = pEnt;
	bool bHookedAlready = false;

	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		if (g_Hooks[i].pVar == pProp)
		{
			hook.pRealProxy = g_Hooks[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}

	if (!bHookedAlready)
		hook.pRealProxy = pProp->GetProxyFn();

	hook.PropType = propType;
	hook.pVar = pProp;
	
	//if this prop has been hooked already, don't set the proxy again
	if (bHookedAlready)
	{
		if (g_SendProxyManager.AddHookToList(hook))
			return (cell_t)1;

		return (cell_t)0;
	}

	if (g_SendProxyManager.AddHookToList(hook))
	{
		pProp->SetProxyFn(GlobalProxy);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_Unhook(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * propName;
	pContext->LocalToString(params[2], &propName);
	CHECK_VALID_ENTITY_SENDPROP(entity, propName)

	IPluginFunction * pFunction = pContext->GetFunctionById(params[3]);
	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		// we check callback here, so, we do not need to check owner
		if (entity == g_Hooks[i].objectID && 
			g_Hooks[i].pEnt == pEnt && 
			//g_Hooks[i].sCallbackInfo.pOwner == (void *)pContext &&
			g_Hooks[i].pVar == info.prop &&
			g_Hooks[i].sCallbackInfo.iCallbackType == CallBackType::Callback_PluginFunction && 
			strcmp(g_Hooks[i].pVar->GetName(), propName) == 0 && 
			g_Hooks[i].sCallbackInfo.pCallback == (void *)pFunction)
		{
			g_SendProxyManager.UnhookProxy(i);
			return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_HookGameRules(IPluginContext * pContext, const cell_t * params)
{
	char * name;
	CHECK_VALID_GAMERULES_SENDPROP(name)

	PropType propType = static_cast<PropType>(params[2]);
	CHECK_PROP_BASE_DATA_TYPE(pProp, propType)

	SendPropHookGamerules hook;
	hook.sCallbackInfo.pCallback = (void *)pContext->GetFunctionById(params[3]);
	hook.sCallbackInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	hook.sCallbackInfo.pOwner = (void *)pContext;
	bool bHookedAlready = false;

	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		if (g_HooksGamerules[i].pVar == pProp)
		{
			hook.pRealProxy = g_HooksGamerules[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}

	if (!bHookedAlready)
		hook.pRealProxy = pProp->GetProxyFn();

	hook.PropType = propType;
	hook.pVar = pProp;
	
	// if this prop has been hooked already, don't set the proxy again
	if (bHookedAlready)
	{
		if (g_SendProxyManager.AddHookToListGamerules(hook))
			return (cell_t)1;

		return (cell_t)0;
	}

	if (g_SendProxyManager.AddHookToListGamerules(hook))
	{
		pProp->SetProxyFn(GlobalProxyGamerules);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_UnhookGameRules(IPluginContext * pContext, const cell_t * params)
{
	char * propName;
	CHECK_VALID_GAMERULES_SENDPROP(propName)	

	IPluginFunction * pFunction = pContext->GetFunctionById(params[2]);
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		//we check callback here, so, we do not need to check owner
		if (g_HooksGamerules[i].sCallbackInfo.iCallbackType == CallBackType::Callback_PluginFunction && 
			g_HooksGamerules[i].pVar == pProp && 
			strcmp(g_HooksGamerules[i].pVar->GetName(), propName) == 0 && 
			g_HooksGamerules[i].sCallbackInfo.pCallback == (void *)pFunction)
		{
			g_SendProxyManager.UnhookProxyGamerules(i);
			return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_HookArray(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);
	
	int entity = params[1];
	char * propName;
	pContext->LocalToString(params[2], &propName);
	CHECK_VALID_ENTITY_SENDPROP(entity, propName)

	int element = params[3];
	PropType propType = static_cast<PropType>(params[4]);
	SendProp *pProp = NULL;
	CHECK_ARRAYPROP_WITH_BASE_TYPE(element, propName)
	
	SendPropHook hook;
	hook.objectID = entity;
	hook.sCallbackInfo.pCallback = (void *)pContext->GetFunctionById(params[5]);;
	hook.sCallbackInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	hook.sCallbackInfo.pOwner = (void *)pContext;
	hook.pEnt = pEnt;
	hook.Element = element;
	bool bHookedAlready = false;

	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		if (g_Hooks[i].pVar == pProp)
		{
			hook.pRealProxy = g_Hooks[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}

	if (!bHookedAlready)
		hook.pRealProxy = pProp->GetProxyFn();

	hook.PropType = propType;
	hook.pVar = pProp;
	
	if (bHookedAlready)
	{
		if (g_SendProxyManager.AddHookToList(hook))
			return (cell_t)1;

		return (cell_t)0;
	}

	if (g_SendProxyManager.AddHookToList(hook))
	{
		pProp->SetProxyFn(GlobalProxy);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_UnhookArray(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * propName;
	pContext->LocalToString(params[2], &propName);
	CHECK_VALID_ENTITY_SENDPROP(entity, propName)

	int element = params[3];
	SendProp *pProp = NULL;
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, propName)

	IPluginFunction * callback = pContext->GetFunctionById(params[4]);
	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		// we check callback here, so, we do not need to check owner
		if (g_Hooks[i].Element == element && 
			g_Hooks[i].sCallbackInfo.iCallbackType == CallBackType::Callback_PluginFunction && 
			g_Hooks[i].sCallbackInfo.pCallback == (void *)callback && 
			//g_Hooks[i].sCallbackInfo.pOwner == (void *)pContext && 
			!strcmp(g_Hooks[i].pVar->GetName(), pProp->GetName()) &&
			g_Hooks[i].pVar == pProp &&
			g_Hooks[i].objectID == entity &&
			g_Hooks[i].pEnt == pEnt)
		{
			g_SendProxyManager.UnhookProxy(i);
			return (cell_t)1;
		}
	}

	return (cell_t)0;
}


static cell_t Native_HookGamerulesArray(IPluginContext * pContext, const cell_t * params)
{
	char * propName;
	CHECK_VALID_GAMERULES_SENDPROP(propName)

	int element = params[2];
	PropType propType = static_cast<PropType>(params[3]);

	pProp = NULL;
	CHECK_ARRAYPROP_WITH_BASE_TYPE(element, propName)
	
	SendPropHookGamerules hook;
	hook.sCallbackInfo.pCallback = (void *)pContext->GetFunctionById(params[4]);
	hook.sCallbackInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	hook.sCallbackInfo.pOwner = (void *)pContext;
	hook.Element = element;
	bool bHookedAlready = false;

	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		if (g_HooksGamerules[i].pVar == pProp)
		{
			hook.pRealProxy = g_HooksGamerules[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}

	if (!bHookedAlready)
		hook.pRealProxy = pProp->GetProxyFn();

	hook.PropType = propType;
	hook.pVar = pProp;
	
	if (bHookedAlready)
	{
		if (g_SendProxyManager.AddHookToListGamerules(hook))
			return (cell_t)1;

		return (cell_t)0;
	}

	if (g_SendProxyManager.AddHookToListGamerules(hook))
	{
		pProp->SetProxyFn(GlobalProxyGamerules);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_UnhookGamerulesArray(IPluginContext * pContext, const cell_t * params)
{
	char * propName;
	CHECK_VALID_GAMERULES_SENDPROP(propName)

	int element = params[2];

	pProp = NULL;
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, propName)

	IPluginFunction * pFunction = pContext->GetFunctionById(params[3]);
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		if (g_HooksGamerules[i].Element == element && 
			g_HooksGamerules[i].pVar == pProp && 
			g_HooksGamerules[i].sCallbackInfo.iCallbackType == CallBackType::Callback_PluginFunction && 
			g_HooksGamerules[i].sCallbackInfo.pCallback == (void *)pFunction && 
			g_HooksGamerules[i].sCallbackInfo.pOwner == (void *)pContext &&
			!strcmp(g_HooksGamerules[i].pVar->GetName(), pProp->GetName()))
		{
			g_SendProxyManager.UnhookProxyGamerules(i);
			return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_IsHooked(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * name;
	pContext->LocalToString(params[2], &name);
	CHECK_VALID_ENTITY_SENDPROP(entity, name)

	SendProp *pProp = info.prop;
	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		if (g_Hooks[i].objectID == entity && 
			g_Hooks[i].pEnt == pEnt &&
			g_Hooks[i].pVar == pProp && 
			g_Hooks[i].sCallbackInfo.pOwner == (void *)pContext && 
			strcmp(name, g_Hooks[i].pVar->GetName()) == 0)
		{
			return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_IsGameRulesHooked(IPluginContext * pContext, const cell_t * params)
{
	char * propName;
	CHECK_VALID_GAMERULES_SENDPROP(propName)

	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		if (g_HooksGamerules[i].sCallbackInfo.pOwner == (void *)pContext && 
			strcmp(propName, g_HooksGamerules[i].pVar->GetName()) == 0 &&
			g_HooksGamerules[i].pVar == pProp)
		{
			return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_IsArrayHooked(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * propName;
	pContext->LocalToString(params[2], &propName);
	CHECK_VALID_ENTITY_SENDPROP(entity, propName)

	int element = params[3];
	PropType propType = static_cast<PropType>(params[4]);
	SendProp *pProp = NULL;
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, propName)

	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		if (g_Hooks[i].sCallbackInfo.pOwner == (void *)pContext && 
			g_Hooks[i].objectID == entity && 
			g_Hooks[i].Element == element && 
			g_Hooks[i].pVar == pProp && 
			strcmp(pProp->GetName(), g_Hooks[i].pVar->GetName()) == 0)
		{
			return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_IsGameRulesArrayHooked(IPluginContext * pContext, const cell_t * params)
{
	char * propName;
	CHECK_VALID_GAMERULES_SENDPROP(propName)

	int element = params[2];
	PropType propType = static_cast<PropType>(params[3]);

	pProp = NULL;
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, propName)

	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		if (g_HooksGamerules[i].sCallbackInfo.pOwner == (void *)pContext && 
			g_HooksGamerules[i].Element == element && 
			g_HooksGamerules[i].pVar == pProp &&
			strcmp(pProp->GetName(), g_HooksGamerules[i].pVar->GetName()) == 0)
		{
			return (cell_t)1;
		}

	}

	return (cell_t)0;
}
static cell_t Native_HookChange(IPluginContext * pContext, const cell_t * params)
{
	bool bSafeCheck = params[0] >= 4;

	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * name;
	pContext->LocalToString(params[2], &name);
	CHECK_VALID_ENTITY_SENDPROP(entity, name)

	SendProp * pProp = info.prop;
	IPluginFunction * callback = nullptr;
	PropType propType = PropType::Prop_Max;
	if (bSafeCheck)
	{
		propType = static_cast<PropType>(params[3]);
		callback = pContext->GetFunctionById(params[4]);
	}
	else
	{
		callback = pContext->GetFunctionById(params[3]);
	}

	if (bSafeCheck)
	{
		CHECK_PROP_BASE_DATA_TYPE(pProp, propType)
	}

	PropChangeHook hook;
	hook.objectID = entity;
	hook.Offset = info.actual_offset;
	hook.pVar = pProp;
	hook.PropType = propType;
	hook.SendPropType = info.prop->GetType();
	CallBackInfo sCallInfo;
	sCallInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	sCallInfo.pCallback = (void *)callback;
	sCallInfo.pOwner = (void *)pContext;

	if (!g_SendProxyManager.AddChangeHookToList(hook, &sCallInfo))
		return pContext->ThrowNativeError("Failed to hook entity %d with prop %s on this callback: this combination may already hooked.", entity, name);

	return (cell_t)1;
}

static cell_t Native_UnhookChange(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * name;
	pContext->LocalToString(params[2], &name);
	CHECK_VALID_ENTITY_SENDPROP(entity, name)

	IPluginFunction * callback = pContext->GetFunctionById(params[3]);

	PropChangeHook hook;
	hook.objectID = entity;
	hook.pVar = info.prop;
	hook.Offset = info.actual_offset;
	hook.SendPropType = info.prop->GetType();

	int i = g_ChangeHooks.Find(hook);
	if (g_ChangeHooks.IsValidIndex(i))
	{
		CallBackInfo sInfo;
		sInfo.pCallback = (void *)callback;
		sInfo.pOwner = (void *)pContext;
		sInfo.iCallbackType = CallBackType::Callback_PluginFunction;
		g_SendProxyManager.UnhookChange(i, &sInfo);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_HookGameRulesChange(IPluginContext * pContext, const cell_t * params)
{
	bool bSafeCheck = params[0] >= 3;

	char * name;
	CHECK_VALID_GAMERULES_SENDPROP(name)

	IPluginFunction * callback = nullptr;
	PropType propType = PropType::Prop_Max;
	if (bSafeCheck)
	{
		propType = static_cast<PropType>(params[2]);
		callback = pContext->GetFunctionById(params[3]);
	}
	else
	{
		callback = pContext->GetFunctionById(params[2]);
	}

	if (bSafeCheck)
	{
		CHECK_PROP_BASE_DATA_TYPE(pProp, propType)
	}

	PropChangeHookGamerules hook;
	hook.Offset = info.actual_offset;
	hook.pVar = pProp;
	hook.PropType = propType;
	hook.SendPropType = info.prop->GetType();
	CallBackInfo sCallInfo;
	sCallInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	sCallInfo.pCallback = (void *)callback;
	sCallInfo.pOwner = (void *)pContext;

	if (!g_SendProxyManager.AddChangeHookToListGamerules(hook, &sCallInfo))
		return pContext->ThrowNativeError("Failed to hook prop %s with this callback: This combination may already hooked.", name);

	return (cell_t)1;
}

static cell_t Native_UnhookGameRulesChange(IPluginContext * pContext, const cell_t * params)
{
	char * name;
	CHECK_VALID_GAMERULES_SENDPROP(name)

	IPluginFunction * callback = pContext->GetFunctionById(params[2]);

	PropChangeHookGamerules hook;
	hook.pVar = info.prop;
	hook.Offset = info.actual_offset;
	hook.SendPropType = info.prop->GetType();

	int i = g_ChangeHooksGamerules.Find(hook);
	if (g_ChangeHooksGamerules.IsValidIndex(i))
	{
		CallBackInfo sInfo;
		sInfo.pCallback = (void *)callback;
		sInfo.pOwner = (void *)pContext;
		sInfo.iCallbackType = CallBackType::Callback_PluginFunction;
		g_SendProxyManager.UnhookChangeGamerules(i, &sInfo);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_HookArrayChange(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * name;
	pContext->LocalToString(params[2], &name);
	CHECK_VALID_ENTITY_SENDPROP(entity, name)

	int element = params[3];
	PropType propType = static_cast<PropType>(params[4]);
	SendProp *pProp = NULL;
	CHECK_ARRAYPROP_WITH_BASE_TYPE(element, name)
	
	PropChangeHook hook;
	hook.objectID = entity;
	hook.Offset = info.actual_offset + pProp->GetOffset();
	hook.pVar = pProp;
	hook.Element = element;
	hook.SendPropType = info.prop->GetType();
	hook.PropType = propType;
	CallBackInfo sCallInfo;
	sCallInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	sCallInfo.pCallback = (void *)pContext->GetFunctionById(params[5]);
	sCallInfo.pOwner = (void *)pContext;

	if (!g_SendProxyManager.AddChangeHookToList(hook, &sCallInfo))
		return pContext->ThrowNativeError("Failed to hook entity %d with prop %s on this callback: this combination may already hooked.", entity, name);

	return (cell_t)1;
}

static cell_t Native_UnhookArrayChange(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);
	
	int entity = params[1];
	char * name;
	pContext->LocalToString(params[2], &name);
	CHECK_VALID_ENTITY_SENDPROP(entity, name)

	SendProp *pProp = NULL;
	int element = params[3];
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, name)

	IPluginFunction *callback = pContext->GetFunctionById(params[4]);

	PropChangeHook hook;
	hook.objectID = entity;
	hook.Element = element;
	hook.SendPropType = info.prop->GetType();
	hook.pVar = pProp;
	hook.Offset = info.actual_offset + pProp->GetOffset();

	int i = g_ChangeHooks.Find(hook);
	if (g_ChangeHooks.IsValidIndex(i))
	{
		CallBackInfo sInfo;
		sInfo.pCallback = (void *)callback;
		sInfo.pOwner = (void *)pContext;
		sInfo.iCallbackType = CallBackType::Callback_PluginFunction;
		g_SendProxyManager.UnhookChange(i, &sInfo);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_HookGameRulesArrayChange(IPluginContext * pContext, const cell_t * params)
{
	char * name;
	CHECK_VALID_GAMERULES_SENDPROP(name)
	
	int element = params[2];
	PropType propType = static_cast<PropType>(params[3]);

	pProp = NULL;
	CHECK_ARRAYPROP_WITH_BASE_TYPE(element, name)

	PropChangeHookGamerules hook;
	hook.Offset = info.actual_offset + pProp->GetOffset();
	hook.pVar = pProp;
	hook.PropType = propType;
	hook.Element = element;
	hook.SendPropType = info.prop->GetType();
	CallBackInfo sCallInfo;
	sCallInfo.iCallbackType = CallBackType::Callback_PluginFunction;
	sCallInfo.pCallback = (void *)pContext->GetFunctionById(params[4]);
	sCallInfo.pOwner = (void *)pContext;

	if (!g_SendProxyManager.AddChangeHookToListGamerules(hook, &sCallInfo))
		return pContext->ThrowNativeError("Failed to hook prop %s with this callback: This combination may already hooked.", name);

	return (cell_t)1;
}

static cell_t Native_UnhookGameRulesArrayChange(IPluginContext * pContext, const cell_t * params)
{
	char * name;
	CHECK_VALID_GAMERULES_SENDPROP(name)
	
	int element = params[2];
	pProp = NULL;
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, name)
	
	IPluginFunction * callback = pContext->GetFunctionById(params[3]);

	PropChangeHookGamerules hook;
	hook.pVar = pProp;
	hook.Element = element;
	hook.SendPropType = info.prop->GetType();
	hook.Offset = info.actual_offset + pProp->GetOffset();

	int i = g_ChangeHooksGamerules.Find(hook);
	if (g_ChangeHooksGamerules.IsValidIndex(i))
	{
		CallBackInfo sInfo;
		sInfo.pCallback = (void *)callback;
		sInfo.pOwner = (void *)pContext;
		sInfo.iCallbackType = CallBackType::Callback_PluginFunction;
		g_SendProxyManager.UnhookChangeGamerules(i, &sInfo);
		return (cell_t)1;
	}

	return (cell_t)0;
}

static cell_t Native_IsChangeHooked(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);

	int entity = params[1];
	char * propName;
	pContext->LocalToString(params[2], &propName);
	CHECK_VALID_ENTITY_SENDPROP(entity, propName)

	PropChangeHook hook;
	hook.objectID = entity;
	hook.SendPropType = info.prop->GetType();
	hook.pVar = info.prop;
	hook.Offset = info.actual_offset;
	if (g_ChangeHooks.HasElement(hook))
	{
		for (int i = 0; i < g_ChangeHooks.Count(); i++)
		{
			if (g_ChangeHooks[i].HasCallBack(pContext))
				return (cell_t)1;
		}
	}
	
	return (cell_t)0;
}

static cell_t Native_IsGameRulesChangeHooked(IPluginContext * pContext, const cell_t * params)
{
	char * propName;
	CHECK_VALID_GAMERULES_SENDPROP(propName)

	PropChangeHookGamerules hook;
	hook.pVar = pProp;
	hook.Offset = info.actual_offset;
	hook.SendPropType = info.prop->GetType();
	if (g_ChangeHooksGamerules.HasElement(hook))
	{
		for (int i = 0; i < g_ChangeHooksGamerules.Count(); i++)
		{
			if (g_ChangeHooksGamerules[i].HasCallBack(pContext))
				return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_IsArrayChangeHooked(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
		return pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);
	
	int entity = params[1];
	char * name;
	pContext->LocalToString(params[2], &name);
	CHECK_VALID_ENTITY_SENDPROP(entity, name)

	int element = params[3];
	SendProp *pProp = NULL;
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, name)

	PropChangeHook hook;
	hook.objectID = entity;
	hook.Element = element;
	hook.SendPropType = info.prop->GetType();
	hook.pVar = pProp;
	hook.Offset = info.actual_offset + pProp->GetOffset();
	if (g_ChangeHooks.HasElement(hook))
	{
		for (int i = 0; i < g_ChangeHooks.Count(); i++)
		{
			if (g_ChangeHooks[i].HasCallBack(pContext))
				return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_IsGameRulesArrayChangeHooked(IPluginContext * pContext, const cell_t * params)
{
	char * name;
	CHECK_VALID_GAMERULES_SENDPROP(name)
	
	int element = params[2];
	pProp = NULL;
	CHECK_ARRAYPROP_NO_BASE_TYPE(element, name)

	PropChangeHookGamerules hook;
	hook.pVar = pProp;
	hook.Element = element;
	hook.SendPropType = info.prop->GetType();
	hook.Offset = info.actual_offset + pProp->GetOffset();
	if (g_ChangeHooksGamerules.HasElement(hook))
	{
		for (int i = 0; i < g_ChangeHooksGamerules.Count(); i++)
		{
			if (g_ChangeHooksGamerules[i].HasCallBack(pContext))
				return (cell_t)1;
		}
	}

	return (cell_t)0;
}

static cell_t Native_GetEntSendPropFlag(IPluginContext * pContext, const cell_t * params)
{
	if (params[1] < 0 || params[1] >= g_iEdictCount)
	{
		pContext->ThrowNativeError("Invalid Edict Index %d", params[1]);
		return (cell_t)-1;
	}

	int entity = params[1];
	char * name;
	pContext->LocalToString(params[2], &name);
	edict_t * pEnt = gamehelpers->EdictOfIndex(entity);
	ServerClass * sc = pEnt->GetNetworkable()->GetServerClass();
	
	if (!sc)
	{
		pContext->ThrowNativeError("Cannot find ServerClass for entity %d", entity);
		return (cell_t)-1;
	}

	sm_sendprop_info_t info;
	gamehelpers->FindSendPropInfo(sc->GetName(), name, &info);
	SendProp * pProp = info.prop;

	if (!pProp)
	{
		pContext->ThrowNativeError("Could not find prop %s", name);
		return (cell_t)-1;
	}
		
	return (cell_t)pProp->GetFlags();
}

static cell_t Native_GetGameRulesSendPropFlag(IPluginContext * pContext, const cell_t * params)
{
	char * name;
	pContext->LocalToString(params[1], &name);
	sm_sendprop_info_t info;

	gamehelpers->FindSendPropInfo(g_szGameRulesProxy, name, &info);
	SendProp * pProp = info.prop;

	if (!pProp)
	{
		pContext->ThrowNativeError("Could not find prop %s", name);
		return (cell_t)-1;
	}

	return (cell_t)pProp->GetFlags();
}

const sp_nativeinfo_t g_MyNatives[] = {
	// methodmap syntax support.
	{"SendProxyManager.Hook", Native_Hook},
	{"SendProxyManager.Unhook", Native_Unhook},

	{"SendProxyManager.HookGameRules", Native_HookGameRules},
	{"SendProxyManager.UnhookGameRules", Native_UnhookGameRules},

	{"SendProxyManager.HookArray", Native_HookArray},
	{"SendProxyManager.UnhookArray", Native_UnhookArray},

	{"SendProxyManager.HookGamerulesArray", Native_HookGamerulesArray},
	{"SendProxyManager.UnhookGamerulesArray", Native_UnhookGamerulesArray},
	
	{"SendProxyManager.IsHooked", Native_IsHooked},
	{"SendProxyManager.IsGameRulesHooked", Native_IsGameRulesHooked},
	{"SendProxyManager.IsArrayHooked", Native_IsArrayHooked},
	{"SendProxyManager.IsGamerulesArrayHooked", Native_IsGameRulesArrayHooked},

	{"SendProxyManager.HookChange", Native_HookChange},
	{"SendProxyManager.UnhookChange", Native_UnhookChange},

	{"SendProxyManager.HookGameRulesChange", Native_HookGameRulesChange},
	{"SendProxyManager.UnhookGameRulesChange", Native_UnhookGameRulesChange},

	{"SendProxyManager.HookArrayChange", Native_HookArrayChange},
	{"SendProxyManager.UnhookArrayChange", Native_UnhookArrayChange},

	{"SendProxyManager.HookGameRulesArrayChange", Native_HookGameRulesArrayChange},
	{"SendProxyManager.UnhookGameRulesArrayChange", Native_UnhookGameRulesArrayChange},

	{"SendProxyManager.IsChangeHooked", Native_IsChangeHooked},
	{"SendProxyManager.IsGameRulesChangeHooked", Native_IsGameRulesChangeHooked},
	{"SendProxyManager.IsArrayChangeHooked", Native_IsArrayChangeHooked},
	{"SendProxyManager.IsGameRulesArrayChangeHooked", Native_IsGameRulesArrayChangeHooked},

	{"GetEntSendPropFlag", Native_GetEntSendPropFlag},
	{"GetGameRulesSendPropFlag", Native_GetGameRulesSendPropFlag},

	// traditional syntax.
	{"SendProxy_Hook", Native_Hook},
	{"SendProxy_HookGameRules", Native_HookGameRules},
	{"SendProxy_HookArrayProp", Native_HookArray},
	{"SendProxy_UnhookArrayProp", Native_UnhookArray},
	{"SendProxy_Unhook", Native_Unhook},
	{"SendProxy_UnhookGameRules", Native_UnhookGameRules},
	{"SendProxy_IsHooked", Native_IsHooked},
	{"SendProxy_IsHookedGameRules", Native_IsGameRulesHooked},
	{"SendProxy_HookPropChange", Native_HookChange},
	{"SendProxy_HookPropChangeGameRules", Native_HookGameRulesChange},
	{"SendProxy_UnhookPropChange", Native_UnhookChange},
	{"SendProxy_UnhookPropChangeGameRules", Native_UnhookGameRulesChange},
	{"SendProxy_HookArrayPropGamerules", Native_HookGamerulesArray},
	{"SendProxy_UnhookArrayPropGamerules", Native_UnhookGamerulesArray},
	{"SendProxy_IsHookedArrayProp", Native_IsArrayHooked},
	{"SendProxy_IsHookedArrayPropGamerules", Native_IsGameRulesArrayHooked},
	{"SendProxy_HookPropChangeArray", Native_HookArrayChange},
	{"SendProxy_UnhookPropChangeArray", Native_UnhookArrayChange},
	{"SendProxy_HookPropChangeArrayGameRules", Native_HookGameRulesArrayChange},
	{"SendProxy_UnhookPropChangeArrayGameRules", Native_UnhookGameRulesArrayChange},
	{"SendProxy_IsPropChangeHooked", Native_IsChangeHooked},
	{"SendProxy_IsPropChangeHookedGameRules", Native_IsGameRulesChangeHooked},
	{"SendProxy_IsPropChangeArrayHooked", Native_IsArrayChangeHooked},
	{"SendProxy_IsPropChangeArrayHookedGameRules", Native_IsGameRulesArrayChangeHooked},
	{"SendProxy_HookPropChangeSafe", Native_HookChange},
	{"SendProxy_HookPropChangeGameRulesSafe", Native_HookGameRulesChange},
	//Probably add listeners for plugins?
	{NULL,	NULL}
};