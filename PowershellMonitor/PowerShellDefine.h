#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <winternl.h>
#include <variant>
#include "mycorinfo.h"

using pObject = PUCHAR;

struct CEEInfoIndexs
{
    DWORD dwUnkown;
    DWORD dwICorMethodInfo_Hack_Index;
    DWORD dwCEEJitInfo_Index;
};
struct ICorJitInfo
{
    PUCHAR vTable;
    PUCHAR vBTable;
};

using getJit_Type = PVOID (__fastcall*)();

using compileMethod_def =  int(__stdcall*)(ULONG_PTR classthis, ICorJitInfo* comp,
    CORINFO_METHOD_INFO* info, unsigned flags,
    BYTE** nativeEntry, ULONG* nativeSizeOfCode);

using getMethodName_def = const char* (__stdcall*)(
    ICorJitInfo* com,
    CORINFO_METHOD_HANDLE ftn,
    const char** moduleName);

using findMethod_def = CORINFO_METHOD_HANDLE(__stdcall*)(
    ICorJitInfo* com,
    CORINFO_MODULE_HANDLE module,
    unsigned metaTOK,
    CORINFO_CONTEXT_HANDLE context);

struct JIT
{
    compileMethod_def compileMethod;
};

struct ICorMethodInfo_Hack_VTable
{
    getMethodName_def getMethodName;
};

struct ICorMethodInfo_Hack
{
    ICorMethodInfo_Hack_VTable* VTable;
};

struct CEEJitInfo_VTable
{
    PUCHAR findClass;
    PUCHAR findField;
    findMethod_def findMethod;
};

struct CEEJitInfo
{
    CEEJitInfo_VTable* VTable;
};

struct System_string
{
    PUCHAR MethodTable;
    ULONG length;
    BYTE buffer[1];
};

struct Binary_value
{
    PUCHAR MethodTable;
    ULONG_PTR length;
    BYTE buffer[1];
};

struct Num_value
{
    PUCHAR MethodTable;
    int value;
};

struct Point_Value
{
    PUCHAR MethodTable;
    PUCHAR value;
};

struct System_stringV2
{
    PUCHAR MethodTable;
    ULONG arrayLength;
    ULONG stringLength;
    BYTE buffer[1];
};

struct CallInfo
{
    PUCHAR MethodTable;
    PUCHAR argnames;
    ULONG argcount;
};

struct Binder
{
    PUCHAR MethodTable;
    PUCHAR Cache;
    System_string* name;
    CallInfo* callinfo;
};

struct CallSite
{
    PUCHAR MethodTable;
    Binder* binder;
};

struct DynamicIns
{
    PUCHAR MethodTable;
    CallSite* site;
};

struct InterpretedFrameData
{
    PUCHAR MethodTable;
    ULONG length;
    PUCHAR first[1];
};

struct InterpretedFrameDataV3
{
    PUCHAR MethodTable;
    ULONG length;
    PUCHAR Unknow;
    PUCHAR first[1];
};

struct Objects
{
    PUCHAR MethodTable;
    ULONG length;
    pObject first[1];
};

struct Interpreter
{
    PUCHAR MethodTable;
    PUCHAR _labelMapping;
    PUCHAR _closureVariables;
    Objects* _objects;
    PUCHAR _labels;
    System_string* _name;
    PUCHAR _debugInfos;
};

struct InterpretedFrame
{
    PUCHAR MethodTable;
    Interpreter* _interpreter;
    PUCHAR _parent;
    PUCHAR _continuations;
    PUCHAR _pendingValue;
    InterpretedFrameData* Data;
    PUCHAR Closure;
    PUCHAR CurrentAbortHandler;
    ULONG _continuationIndex;
    ULONG _pendingContinuation;
    ULONG StackIndex;
};

struct InterpretedFrameV3
{
    PUCHAR MethodTable;
    Interpreter* _interpreter;
    PUCHAR _parent;
    PUCHAR _continuations;
    PUCHAR _pendingValue;
    InterpretedFrameDataV3* Data;
    PUCHAR Closure;
    PUCHAR CurrentAbortHandler;
    ULONG _continuationIndex;
    ULONG _pendingContinuation;
    ULONG StackIndex;
};

struct PositionHelper
{
    PUCHAR MethodTable;
    System_string* _filename;
    System_string* _scriptText;
    PUCHAR _lineStartMap;
};

struct InternalScriptExtent
{
    PUCHAR MethodTable;
    PositionHelper* _positionHelper;
    ULONG _startOffset;
    ULONG _endOffset;
};

struct ScriptExtent
{
    PUCHAR MethodTable;
    ULONG length;
    PUCHAR first[1];
};

struct FunctionContext
{
    PUCHAR MethodTable;
    PUCHAR _scriptBlock;
    System_string* _file;
    ScriptExtent* _sequencePoints;
    PUCHAR _executionContext;
};

struct NodeToken
{
    PUCHAR MethodTable;
    System_stringV2* _text;
    PUCHAR _data;
    System_stringV2* _file;
    System_stringV2* _script;
    System_stringV2* _line;
};

struct MethodCallNode
{
    PUCHAR MethodTable;
    NodeToken* _nodeToken;
    ULONG _flags;
    PUCHAR _target;
    PUCHAR _arguments;
    PUCHAR _typeConstraint;
    ULONG _isStatic;
};

struct ArgumentsArray
{
    PUCHAR MethodTable;
    ULONG length;
    PUCHAR ElementMethodTable;
    PUCHAR first[1];
};

struct MethodInfo
{
    PUCHAR MethodTable;
    PUCHAR _target;
    PUCHAR _methodBase;
    PUCHAR _methodPtr;
    PUCHAR _methodPtrAux;
};

struct PSmethodInfo
{
    PUCHAR MethodTable;
    PUCHAR instance;
    PUCHAR name;
};

struct CommandInfo
{
    PUCHAR MethodTable;
    System_string* name;
};

struct CommandInfoV2
{
    PUCHAR MethodTable;
    System_stringV2* name;
};

struct ArgumentItemV2
{
    PUCHAR MethodTable;
    PUCHAR name;
    System_stringV2* _argument1;
    PUCHAR _argument2;
};

struct Parameter
{
    PUCHAR MethodTable;
    PUCHAR extent;
    System_string* parameterName;
    System_string* parameterText;
};

struct Argument
{
    PUCHAR MethodTable;
    PUCHAR extent;
    System_string* value;
};

struct ArgumentItem
{
    PUCHAR MethodTable;
    Parameter* _parameter;
    Argument* _argument;
    PUCHAR _spaceAfterParameter;
};

struct CommandArgumentItems
{
    PUCHAR MethodTable;
    ULONG length;
    PUCHAR first[1];
};

struct CommandArgumentItemsV2
{
    PUCHAR MethodTable;
    ULONG length;
    PUCHAR ElementMethodTable;
    PUCHAR first[1];
};

struct ItemsList
{
    PUCHAR MethodTable;
    PUCHAR _items;
    PUCHAR _syncRoot;
    ULONG _size;
    ULONG _version;
};

struct CommandArguments
{
    PUCHAR MethodTable;
    ItemsList* items;
    PUCHAR _syncRoot;
};

struct CommandProcessor
{
    PUCHAR MethodTable;
    PUCHAR command;
    CommandInfo* commandinfo;
    PUCHAR commandRuntime;
    PUCHAR context;
    PUCHAR _commandSessionState;
    PUCHAR k__BackingField;
    PUCHAR _previousScope;
    PUCHAR _previousCommandSessionState;
    CommandArguments* arguments;
};

struct CommandProcessorV2
{
    PUCHAR MethodTable;
    PUCHAR command;
    CommandInfoV2* commandinfo;
    PUCHAR commandRuntime;
    PUCHAR parameterBinderController;
    PUCHAR context;
    PUCHAR _commandSessionState;
    PUCHAR _commandScope;
    PUCHAR _previousScope;
    PUCHAR _activationRecord;
    PUCHAR _previousActivationRecord;
    PUCHAR _previousCommandSessionState;
    CommandArguments* arguments;
};

using InvokeMethod_def = pObject(__fastcall*)(MethodCallNode* CallNode, pObject psobject, pObject target, ArgumentsArray* arguments, pObject value);
