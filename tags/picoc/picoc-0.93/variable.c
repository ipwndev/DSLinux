#include "picoc.h"

/* the table of global definitions */
struct Table GlobalTable;
struct TableEntry *GlobalHashTable[GLOBAL_TABLE_SIZE];

/* the table of string literal values */
struct Table StringLiteralTable;
struct TableEntry *StringLiteralHashTable[STRING_LITERAL_TABLE_SIZE];

/* the stack */
struct StackFrame *TopStackFrame = NULL;


/* initialise the variable system */
void VariableInit()
{
    TableInitTable(&GlobalTable, &GlobalHashTable[0], GLOBAL_TABLE_SIZE, TRUE);
    TableInitTable(&StringLiteralTable, &StringLiteralHashTable[0], STRING_LITERAL_TABLE_SIZE, TRUE);
    TopStackFrame = NULL;
}

/* deallocate the contents of a variable */
void VariableFree(struct Value *Val)
{
    if (Val->ValOnHeap)
    {
        /* free function bodies */
        if (Val->Typ == &FunctionType && Val->Val->FuncDef.Intrinsic == NULL)
            HeapFreeMem((void *)Val->Val->FuncDef.Body.Pos);

        /* free macro bodies */
        if (Val->Typ == &MacroType)
            HeapFreeMem((void *)Val->Val->Parser.Pos);

        /* free the value */
        HeapFreeMem(Val);
    }
}

/* deallocate the global table and the string literal table */
void VariableTableCleanup(struct Table *HashTable)
{
    struct TableEntry *Entry;
    struct TableEntry *NextEntry;
    int Count;
    
    for (Count = 0; Count < HashTable->Size; Count++)
    {
        for (Entry = HashTable->HashTable[Count]; Entry != NULL; Entry = NextEntry)
        {
            NextEntry = Entry->Next;
            VariableFree(Entry->p.v.Val);
                
            /* free the hash table entry */
            HeapFreeMem(Entry);
        }
    }
}

void VariableCleanup()
{
    VariableTableCleanup(&GlobalTable);
    VariableTableCleanup(&StringLiteralTable);
}

/* allocate some memory, either on the heap or the stack and check if we've run out */
void *VariableAlloc(struct ParseState *Parser, int Size, int OnHeap)
{
    void *NewValue;
    
    if (OnHeap)
        NewValue = HeapAllocMem(Size);
    else
        NewValue = HeapAllocStack(Size);
    
    if (NewValue == NULL)
        ProgramFail(Parser, "out of memory");
    
#ifdef DEBUG_HEAP
    if (!OnHeap)
        printf("pushing %d at 0x%lx\n", Size, (unsigned long)NewValue);
#endif
        
    return NewValue;
}

/* allocate a value either on the heap or the stack using space dependent on what type we want */
struct Value *VariableAllocValueAndData(struct ParseState *Parser, int DataSize, int IsLValue, struct Value *LValueFrom, int OnHeap)
{
    struct Value *NewValue = VariableAlloc(Parser, MEM_ALIGN(sizeof(struct Value)) + DataSize, OnHeap);
    NewValue->Val = (union AnyValue *)((char *)NewValue + MEM_ALIGN(sizeof(struct Value)));
    NewValue->ValOnHeap = OnHeap;
    NewValue->ValOnStack = !OnHeap;
    NewValue->IsLValue = IsLValue;
    NewValue->LValueFrom = LValueFrom;
    
    return NewValue;
}

/* allocate a value given its type */
struct Value *VariableAllocValueFromType(struct ParseState *Parser, struct ValueType *Typ, int IsLValue, struct Value *LValueFrom, int OnHeap)
{
    int Size = TypeSize(Typ, Typ->ArraySize, FALSE);
    struct Value *NewValue = VariableAllocValueAndData(Parser, Size, IsLValue, LValueFrom, OnHeap);
    assert(Size > 0 || Typ == &VoidType);
    NewValue->Typ = Typ;
    
    return NewValue;
}

/* allocate a value either on the heap or the stack and copy its value */
struct Value *VariableAllocValueAndCopy(struct ParseState *Parser, struct Value *FromValue, int OnHeap)
{
    int CopySize = TypeSizeValue(FromValue);
    struct Value *NewValue = VariableAllocValueAndData(Parser, CopySize, FromValue->IsLValue, FromValue->LValueFrom, OnHeap);
    NewValue->Typ = FromValue->Typ;
    memcpy((void *)NewValue->Val, (void *)FromValue->Val, CopySize);
    
    return NewValue;
}

/* allocate a value either on the heap or the stack from an existing AnyValue and type */
struct Value *VariableAllocValueFromExistingData(struct ParseState *Parser, struct ValueType *Typ, union AnyValue *FromValue, int IsLValue, struct Value *LValueFrom)
{
    struct Value *NewValue = VariableAlloc(Parser, sizeof(struct Value), FALSE);
    NewValue->Typ = Typ;
    NewValue->Val = FromValue;
    NewValue->ValOnHeap = FALSE;
    NewValue->ValOnStack = FALSE;
    NewValue->IsLValue = IsLValue;
    NewValue->LValueFrom = LValueFrom;
    
    return NewValue;
}

/* allocate a value either on the heap or the stack from an existing Value, sharing the value */
struct Value *VariableAllocValueShared(struct ParseState *Parser, struct Value *FromValue)
{
    return VariableAllocValueFromExistingData(Parser, FromValue->Typ, FromValue->Val, FromValue->IsLValue, FromValue->IsLValue ? FromValue : NULL);
}

/* define a variable. Ident must be registered */
struct Value *VariableDefine(struct ParseState *Parser, char *Ident, struct Value *InitValue, struct ValueType *Typ, int MakeWritable)
{
    struct Value *AssignValue;
    
    if (InitValue != NULL)
        AssignValue = VariableAllocValueAndCopy(Parser, InitValue, TopStackFrame == NULL);
    else
        AssignValue = VariableAllocValueFromType(Parser, Typ, MakeWritable, NULL, TopStackFrame == NULL);
    
    AssignValue->IsLValue = MakeWritable;
        
    if (!TableSet((TopStackFrame == NULL) ? &GlobalTable : &TopStackFrame->LocalTable, Ident, AssignValue))
        ProgramFail(Parser, "'%s' is already defined", Ident);
    
    return AssignValue;
}

/* check if a variable with a given name is defined. Ident must be registered */
int VariableDefined(const char *Ident)
{
    struct Value *FoundValue;
    
    if (TopStackFrame == NULL || !TableGet(&TopStackFrame->LocalTable, Ident, &FoundValue))
    {
        if (!TableGet(&GlobalTable, Ident, &FoundValue))
            return FALSE;
    }

    return TRUE;
}

/* get the value of a variable. must be defined. Ident must be registered */
void VariableGet(struct ParseState *Parser, const char *Ident, struct Value **LVal)
{
    if (TopStackFrame == NULL || !TableGet(&TopStackFrame->LocalTable, Ident, LVal))
    {
        if (!TableGet(&GlobalTable, Ident, LVal))
            ProgramFail(Parser, "'%s' is undefined", Ident);
    }
}

/* define a global variable shared with a platform global. Ident will be registered */
void VariableDefinePlatformVar(struct ParseState *Parser, char *Ident, struct ValueType *Typ, union AnyValue *FromValue, int IsWritable)
{
    struct Value *SomeValue = VariableAllocValueAndData(NULL, 0, IsWritable, NULL, TRUE);
    SomeValue->Typ = Typ;
    if (Typ->Base != TypeArray)
        SomeValue->Val = FromValue;
    
    if (!TableSet((TopStackFrame == NULL) ? &GlobalTable : &TopStackFrame->LocalTable, TableStrRegister(Ident), SomeValue))
        ProgramFail(Parser, "'%s' is already defined", Ident);
}

/* free and/or pop the top value off the stack. Var must be the top value on the stack! */
void VariableStackPop(struct ParseState *Parser, struct Value *Var)
{
    int Success;
    
#ifdef DEBUG_HEAP
    if (Var->ValOnStack)
        printf("popping %ld at 0x%lx\n", (unsigned long)(sizeof(struct Value) + TypeSizeValue(Var)), (unsigned long)Var);
#endif
        
    if (Var->ValOnHeap)
    { 
        if (Var->Val != NULL)
            HeapFreeMem(Var->Val);
            
        Success = HeapPopStack(Var, sizeof(struct Value));                       /* free from heap */
    }
    else if (Var->ValOnStack)
        Success = HeapPopStack(Var, sizeof(struct Value) + TypeSizeValue(Var));  /* free from stack */
    else
        Success = HeapPopStack(Var, sizeof(struct Value));                       /* value isn't our problem */
        
    if (!Success)
        ProgramFail(Parser, "stack underrun");
}

/* add a stack frame when doing a function call */
void VariableStackFrameAdd(struct ParseState *Parser, int NumParams)
{
    struct StackFrame *NewFrame;
    
    HeapPushStackFrame();
    NewFrame = HeapAllocStack(sizeof(struct StackFrame) + sizeof(struct Value *) * NumParams);
    if (NewFrame == NULL)
        ProgramFail(Parser, "out of memory");
        
    NewFrame->ReturnParser = *Parser;
    NewFrame->Parameter = (NumParams > 0) ? ((void *)((char *)NewFrame + sizeof(struct StackFrame))) : NULL;
    TableInitTable(&NewFrame->LocalTable, &NewFrame->LocalHashTable[0], LOCAL_TABLE_SIZE, FALSE);
    NewFrame->PreviousStackFrame = TopStackFrame;
    TopStackFrame = NewFrame;
}

/* remove a stack frame */
void VariableStackFramePop(struct ParseState *Parser)
{
    if (TopStackFrame == NULL)
        ProgramFail(Parser, "stack is empty - can't go back");
        
    *Parser = TopStackFrame->ReturnParser;
    TopStackFrame = TopStackFrame->PreviousStackFrame;
    HeapPopStackFrame();
}

/* get a string literal. assumes that Ident is already registered. NULL if not found */
struct Value *VariableStringLiteralGet(char *Ident)
{
    struct Value *LVal = NULL;

    if (TableGet(&StringLiteralTable, Ident, &LVal))
        return LVal;
    else
        return NULL;
}

/* define a string literal. assumes that Ident is already registered */
void VariableStringLiteralDefine(char *Ident, struct Value *Val)
{
    TableSet(&StringLiteralTable, Ident, Val);
}

/* check a pointer for validity and dereference it for use */
void *VariableDereferencePointer(struct ParseState *Parser, struct Value *PointerValue, struct Value **DerefVal, int *DerefOffset, struct ValueType **DerefType, int *DerefIsLValue)
{
    if (DerefVal != NULL)
        *DerefVal = NULL;
        
    if (DerefType != NULL)
        *DerefType = PointerValue->Typ->FromType;
        
    if (DerefOffset != NULL)
        *DerefOffset = 0;
        
    if (DerefIsLValue != NULL)
        *DerefIsLValue = TRUE;

    return PointerValue->Val->NativePointer;
}

