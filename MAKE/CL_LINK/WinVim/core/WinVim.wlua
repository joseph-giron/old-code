-- vim: ft=lua
require "vk"
require "winvim_core"
-------------------------------------------------------------------------
----------------------------------CORE-----------------------------------
-------------------------------------------------------------------------
wv = winvim_core

wv._ON_KEY_DOWN = {}
wv._ON_KEY_UP = {}
wv._ON_CHAR = {}

wv._CurRotine = nil     --��ǰ��Э��
wv._WaitList = {}       --�ȴ��б�

function wv.RegisterKey(vk, func, ifdown)
    assert(type(vk)=="number", "[ASSERT] The first argument must be a number")
    assert(type(func)=="function", "[ASSERT] The second argument must be a function")
    local down = ifdown or true
    if down then
        wv._ON_KEY_DOWN[vk] = func
    else
        wv._ON_KEY_UP[vk] = func
    end
    return true
end

function wv.RegisterCommand(ch, func)
    assert(type(ch)=="string", "[ASSERT] The first argument must be a string")
    assert(type(func)=="function", "[ASSERT] The second argument must be a function")
    wv._ON_CHAR[ch] = func
    return true
end

local _Count
function wv.DealNumber(ch)
    local c,b1,b9 = ch:byte(),string.byte("1"),string.byte("9")
    if _Count then b1 = b1-1 end   --�Ѿ���ʼ�˽�������
    if c>=b1 and c<=b9 then
       if _Count then
           _Count = _Count..ch
       else
           _Count = ch
       end
       wv.TextOut(_Count)
       return true
    end
    _Count = nil
    return false
end
function wv.GetNumber()
    return tonumber(_Count) or 1
end

--Waitϵ�к���ֻ�ܴӻص������������
function wv.WaitChar(ch)
    table.insert(wv._WaitList, {_type="char", _key=ch, _co=wv._CurRotine})
    return coroutine.yield()
end
function wv.WaitKeyDown(key)
    table.insert(wv._WaitList, {_type="keydown", _key=key, _co=wv._CurRotine})
    return coroutine.yield()
end
function wv.WaitKeyUp(key)
    table.insert(wv._WaitList, {_type="keyup", _key=key, _co=wv._CurRotine})
    return coroutine.yield()
end

function wv.CheckWait(key, Type)
    local wait
    local i = 1
    wait = wv._WaitList[i]
    while wait do
        if wait._type == Type then       --�жϵȴ������Ƿ���ͬ
            local k = wait._key
            if (not k) or k==key then   --�ж������ļ��Ƿ���ͬ
                table.remove(wv._WaitList, i)--�ӵȴ��б����Ƴ�
                coroutine.resume(wait._co, key)--���Ѵ�Э��
                return true
            else
                print("wait return false")
                return false
            end
        end
        i = i + 1
        wait = wv._WaitList[i]
    end
    return false
end

_CALLBACK = function(vkCode, bDown)         --��ͨWinVim����core������,����true �����Ѿ�������
    --wv.TextOut(vk[vkCode])
    local ch = wv.KeyToChar(vkCode)
    local fun, ck
    local wait_type, deal_table

    if bDown then                           
        wait_type = "keydown"
        deal_table = wv._ON_KEY_DOWN
    else
        wait_type = "keyup"
        deal_table = wv._ON_KEY_UP
    end
    --print("callback "..vk[vkCode].." \t"..wait_type)
                                            --�����¼��������ַ��¼�����
    if wv.CheckWait(vkCode, wait_type) then print("Wait return "..wait_type) return true end
    fun = deal_table[vkCode]
    if fun then
        wv._CurRotine = coroutine.create(fun)
        co = wv._CurRotine
        local _, ret = coroutine.resume(co, vkCode) 
        return not ret
    end
                                            --�����ַ��¼�
    if ch ~= "" then
        if not bDown then return true end                  --ֻ����KeyDownʱ����Char
        if ch:byte()<31 then return false end              --�����ַ�
        if wv.CheckWait(ch, "char") then return true end
        if wv.DealNumber(ch) then return true end
        fun = wv._ON_CHAR[ch]
        if fun then
            wv._CurRotine = coroutine.create(fun)
            co = wv._CurRotine
            coroutine.resume(co, wv.GetNumber()) 
        end
        return true
    else
        return false
    end

    return true
end

----------------------------------------------------------------------------
--��꺯����չ
function wv.MouseMoveTo(x,y)
    local y = y or 0
    local cx,cy = wv.GetCursorPos()
    wv.MouseMove(x-cx, y-cy)
end
function wv.MouseClick(key)--����ĳ������
    wv.MouseDown(key)
    wv.MouseUp(key)
end
----------------------------------------------------------------------------
--���̺�����չ
function wv.ClickKey(vkCode)--����ĳ��
    wv.DownKey(vkCode)
    wv.UpKey(vkCode)
end
function wv.CoDownKeys(...)--ͬʱ��������
    local count = #arg
    if count == 0 then return end
    for i=1,count do
        wv.DownKey(arg[i])
    end
    for i=count,1,-1 do
        wv.UpKey(arg[i])
    end
end

----------------------------------------------------------------------------

--�����Զ�������
require "winvimrc"
--frame:Show(true) 
--������Ϣѭ���������Ӳ��񲻵�������Ϣ
wv.MsgLoop()
