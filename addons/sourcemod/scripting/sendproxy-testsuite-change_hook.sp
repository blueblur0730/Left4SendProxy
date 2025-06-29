#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <sendproxy>

// see sendproxy-testsuite-hook.sp for more details.

public void OnPluginStart()
{
    RegConsoleCmd("sm_starthook", Cmd_StartHook);
    RegConsoleCmd("sm_endhook", Cmd_EndHook);
    RegConsoleCmd("sm_checkhook", Cmd_CheckHook);
    RegConsoleCmd("sm_setprop", Cmd_SetProp);
}

Action Cmd_StartHook(int client, int args)
{
    // You can also hook other entities that share this sendprop with one callback. Use iEntity to specify.
    bool b1 = SendProxyManager.HookChange(client, "m_usingMountedGun", Prop_Bool, OnSendPropChanged__m_usingMountedGun);

    // will throw an error, cause its bit != 1.
    //bool b2 = SendProxyManager.HookChange(client, "m_checkpointBoomerBilesUsed", Prop_Bool, OnSendPropChanged__m_checkpointBoomerBilesUsed);

    // okey.
    bool b2 = SendProxyManager.HookChange(client, "m_checkpointBoomerBilesUsed", Prop_Int, OnSendPropChanged__m_checkpointBoomerBilesUsed); 

    // will throw an error, cause this is an DPT_Array prop, with an array prop in the same name. (which is marked flag with SPROP_INSIDEARRAY.)
    //bool b3 = SendProxyManager.HookGameRulesChange("m_szScriptedHUDStringSet", Prop_String, OnSendPropChanged__m_szScriptedHUDStringSet_1);

    bool b3 = SendProxyManager.HookGameRulesArrayChange("m_szScriptedHUDStringSet", 0, Prop_String, OnSendPropChanged__m_szScriptedHUDStringSet_2);
    bool b4 = SendProxyManager.HookGameRulesArrayChange("m_szScriptedHUDStringSet", 1, Prop_String, OnSendPropChanged__m_szScriptedHUDStringSet_2);
    bool b5 = SendProxyManager.HookGameRulesChange("m_bIsDedicatedServer", Prop_Bool, OnSendPropChanged__m_bIsDedicatedServer);
    bool b6 = SendProxyManager.HookGameRulesChange("m_flAccumulatedTime", Prop_Float, OnSendPropChanged__m_flAccumulatedTime);

    // m_iAmmo is an datatable type prop, which contains a table, each elements of the table is a sendprop with ordered number named "000", "001" etc.
    // Any DPT_Datatable or DPT_Array type is used with HookArray.
    bool b7 = SendProxyManager.HookArrayChange(client, "m_iAmmo", 0, Prop_Int, OnSendPropChanged__m_iAmmo);
    bool b8 = SendProxyManager.HookArrayChange(client, "m_iAmmo", 1, Prop_Int, OnSendPropChanged__m_iAmmo); //1, 2, 3...
    
    PrintToServer("Started hook. %d, %d, %d, %d, %d, %d, %d, %d", b1, b2, b3, b4, b5, b6, b7, b8);
    return Plugin_Handled;
}

void OnSendPropChanged__m_usingMountedGun(int iEntity, const char[] cPropName, int iOldValue, int iNewValue, int iElement)
{
    // since this is not a inside array / array / datatable sendprop, the element is 0.
    PrintToServer("OnSendPropChanged__m_usingMountedGun: %d, %s, %d, %d, %d", iEntity, cPropName, iOldValue, iNewValue, iElement);
}

void OnSendPropChanged__m_checkpointBoomerBilesUsed(int iEntity, const char[] cPropName, int iOldValue, int iNewValue, int iElement)
{
    // since this is not a inside array / array / datatable sendprop, the element is 0.
    PrintToServer("OnSendPropChanged__m_checkpointBoomerBilesUsed: %d, %s, %d, %d, %d", iEntity, cPropName, iOldValue, iNewValue, iElement);
}

// since we pass the string to element(slot) 0, the element(slot) 1 will pass nothing.
void OnSendPropChanged__m_szScriptedHUDStringSet_2(const char[] cPropName, const char[] cOldValue, const char[] cNewValue, int iElement)
{
    PrintToServer("OnSendPropChanged__m_szScriptedHUDStringSet_2: %s, %s, %s, %d", cPropName, cOldValue, cNewValue, iElement);
}

void OnSendPropChanged__m_bIsDedicatedServer(const char[] cPropName, int iOldValue, int iNewValue, int iElement)
{
    PrintToServer("OnSendPropChanged__m_bIsDedicatedServer: %s, %d, %d, %d", cPropName, iOldValue, iNewValue, iElement);
}

void OnSendPropChanged__m_flAccumulatedTime(const char[] cPropName, float flOldValue, float flNewValue, int iElement)
{
    PrintToServer("OnSendPropChanged__m_flAccumulatedTime: %s, %.02f, %.02f, %d", cPropName, flOldValue, flNewValue, iElement);
}

void OnSendPropChanged__m_iAmmo(int iEntity, const char[] cPropName, int iOldValue, int iNewValue, int iElement)
{
    // the name would be "000", "001" ... etc.
    PrintToServer("OnSendPropChanged__m_iAmmo: %d, %s, %d, %d, %d", iEntity, cPropName, iOldValue, iNewValue, iElement);
}

Action Cmd_EndHook(int client, int args)
{
    bool b1 = SendProxyManager.UnhookChange(client, "m_usingMountedGun", OnSendPropChanged__m_usingMountedGun);
    bool b2 = SendProxyManager.UnhookChange(client, "m_checkpointBoomerBilesUsed", OnSendPropChanged__m_checkpointBoomerBilesUsed); 
    bool b3 = SendProxyManager.UnhookGameRulesArrayChange("m_szScriptedHUDStringSet", 0, OnSendPropChanged__m_szScriptedHUDStringSet_2);
    bool b4 = SendProxyManager.UnhookGameRulesArrayChange("m_szScriptedHUDStringSet", 1, OnSendPropChanged__m_szScriptedHUDStringSet_2);
    bool b5 = SendProxyManager.UnhookGameRulesChange("m_bIsDedicatedServer", OnSendPropChanged__m_bIsDedicatedServer);
    bool b6 = SendProxyManager.UnhookGameRulesChange("m_flAccumulatedTime", OnSendPropChanged__m_flAccumulatedTime);
    bool b7 = SendProxyManager.UnhookArrayChange(client, "m_iAmmo", 0, OnSendPropChanged__m_iAmmo);
    bool b8 = SendProxyManager.UnhookArrayChange(client, "m_iAmmo", 1, OnSendPropChanged__m_iAmmo);

    PrintToServer("Ended hook. %d, %d, %d, %d, %d, %d, %d, %d", b1, b2, b3, b4, b5, b6, b7, b8);
    return Plugin_Handled;
}

Action Cmd_CheckHook(int client, int args)
{
    PrintToServer("Hooked? %d, %d, %d, %d, %d, %d, %d, %d", 
    SendProxyManager.IsChangeHooked(client, "m_usingMountedGun"),
    SendProxyManager.IsChangeHooked(client, "m_checkpointBoomerBilesUsed"),
    SendProxyManager.IsGameRulesArrayChangeHooked("m_szScriptedHUDStringSet", 0),
    SendProxyManager.IsGameRulesArrayChangeHooked("m_szScriptedHUDStringSet", 1),
    SendProxyManager.IsGameRulesChangeHooked("m_flAccumulatedTime"),
    SendProxyManager.IsGameRulesChangeHooked("m_bIsDedicatedServer"),
    SendProxyManager.IsArrayChangeHooked(client, "m_iAmmo", 0),
    SendProxyManager.IsArrayChangeHooked(client, "m_iAmmo", 1));
    return Plugin_Handled;
}

Action Cmd_SetProp(int client, int args)
{
    int i = GetCmdArgInt(1);
    if (i == 1)
    {
        PrintToServer("Setprop: 1");
        SetEntProp(client, Prop_Send, "m_usingMountedGun", 1);
        SetEntProp(client, Prop_Send, "m_checkpointBoomerBilesUsed", 1);
        SetEntProp(client, Prop_Send, "m_iAmmo", 1, _, 0);
        SetEntProp(client, Prop_Send, "m_iAmmo", 1, _, 1);
        GameRules_SetPropString("m_szScriptedHUDStringSet", "aaa", false, 0);
        GameRules_SetPropString("m_szScriptedHUDStringSet", "bbb", false, 1);
        GameRules_SetProp("m_bIsDedicatedServer", 0);
        GameRules_SetPropFloat("m_flAccumulatedTime", 1.0);
    }
    else if (i == 0)
    {
        PrintToServer("Setprop: 0");
        SetEntProp(client, Prop_Send, "m_usingMountedGun", 0);
        SetEntProp(client, Prop_Send, "m_checkpointBoomerBilesUsed", 0);
        SetEntProp(client, Prop_Send, "m_iAmmo", 0, _, 0);
        SetEntProp(client, Prop_Send, "m_iAmmo", 0, _, 1);
        GameRules_SetPropString("m_szScriptedHUDStringSet", "", false, 0);
        GameRules_SetPropString("m_szScriptedHUDStringSet", "", false, 1);
        GameRules_SetProp("m_bIsDedicatedServer", 1);
        GameRules_SetPropFloat("m_flAccumulatedTime", 0.0);
    }

    return Plugin_Handled;
}