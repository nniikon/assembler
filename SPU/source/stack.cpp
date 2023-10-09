#include "../include/stack.h"


static FILE* stkerr = stderr;


static const char* ErrorString[] = 
{
    ERROR_NAME(GENERATE_STRING)
};

/**
 * @brief Check stack class for errors.
 * 
 * @param[in] stk Stack struct.
 * 
 * @return Error code.
 */
static StackError checkStackError(Stack *stk);

/**
 * @brief Calculates stack hash value.
 * 
 * @param[in] stk Stack struct.
 * 
 * @return Hash value.
 * 
*/
static unsigned long long calculateStackHash(const Stack* stk);


/**
 * @brief Calculates stack.data[] hash value.
 * 
 * @param[in] stk Stack struct.
 * 
 * @return Hash value.
 * 
*/
static unsigned long long calculateDataHash(const Stack* stk);





// TODO: move to .cpp
#ifndef RELEASE
    #define STACK_DUMP(stk, stackError) stackDump_internal((stk), (stackError), __FILE__, __LINE__, __FUNCTION__)
    /**
     * Verifies the stack structure and returns any errors.
     * 
     * @param[in]  stk  The stack structure to be checked.
     */
    #define CHECK_DUMP_AND_RETURN_ERROR(stk)                     \
    do                                                           \
    {                                                            \
        StackError defineError = checkStackError(stk);           \
        if (defineError != NO_ERROR)                             \
        {                                                        \
            STACK_DUMP((stk), defineError);                      \
            return checkStackError((stk));                       \
        }                                                        \
    } while (0) 

    /**
     * Dumps the stack error and returns any errors.
     * 
     * @param[in]  stk   The stack structure to be dumped.
     * @param[in]  error The error to be dumped.
     * 
     * @note Does nothing if `error` = `NO_ERROR`.
     */
    #define DUMP_AND_RETURN_ERROR(stk, error)                  \
    do                                                         \
    {                                                          \
        if ((error) != NO_ERROR)                               \
        {                                                      \
            STACK_DUMP((stk), (error));                        \
            return (error);                                    \
        }                                                      \
    } while (0) 

#else
    #define CHECK_DUMP_AND_RETURN_ERROR(stk)  ;
    #define DUMP_AND_RETURN_ERROR(stk, error) ;
    #define STACK_DUMP(stk, error)            ;
#endif                                          





#ifdef HASH_PROTECT
       
    #define CHECK_STACK_HASH_RETURN_ERROR(stk)                            \
    do                                                                    \
    {                                                                     \
        if ((stk)->structHash != calculateStackHash((stk)))               \
        {                                                                 \
            DUMP_AND_RETURN_ERROR(stk, UNREGISTERED_STRUCT_ACCESS_ERROR); \
        }                                                                 \
    } while (0);
    
    #define CHECK_DATA_HASH_RETURN_ERROR(stk)                             \
    do                                                                    \
    {                                                                     \
        if ((stk)->dataHash != calculateDataHash((stk)))                  \
        {                                                                 \
            DUMP_AND_RETURN_ERROR(stk, UNREGISTERED_DATA_ACCESS_ERROR);   \
        }                                                                 \
    } while (0);
    
    
    #define UPDATE_HASH(stk)                                  \
    do                                                        \
    {                                                         \
        (stk)->structHash = calculateStackHash(stk);          \
        (stk)->dataHash   = calculateDataHash(stk);           \
    } while (0);                                              
    

#else
    #define CHECK_DATA_HASH_RETURN_ERROR(stk)  ;
    #define CHECK_STACK_HASH_RETURN_ERROR(stk) ;
    #define UPDATE_HASH(stk)                   ;
#endif





#define CHECK_CONDITION_RETURN_ERROR(condition, error)        \
do                                                            \
{                                                             \
    if (condition)                                            \
    {                                                         \
        stackDump(stk, error);                                \
        return error;                                         \
    }                                                         \
} while (0)





static StackError checkStackError(Stack *stk)
{
#ifdef CANARY_PROTECT
    if (stk->leftCanary  != CANARY_VALUE) return DEAD_STRUCT_CANARY_ERROR;
    if (stk->rightCanary != CANARY_VALUE) return DEAD_STRUCT_CANARY_ERROR;

    if (*(canary_t*)((size_t)stk->data - sizeof(canary_t)) != CANARY_VALUE) return DEAD_DATA_CANARY_ERROR;
    if (*(canary_t*)(stk->data + stk->capacity) != CANARY_VALUE)            return DEAD_DATA_CANARY_ERROR;
#endif
    if (stk       == NULL)                return STRUCT_NULL_ERROR;
    if (stk->capacity < 0)                return NEGATIVE_CAPACITY_ERROR;
    if (stk->size     < 0)                return NEGATIVE_SIZE_ERROR;
    if (stk->data == NULL)                return DATA_NULL_ERROR;
    if (stk->size > stk->capacity)        return SIZE_CAPACITY_ERROR;
    else      /*POST IRONIYA*/            return NO_ERROR;

}


static StackError increaseCapacity(Stack* stk, const float coef)
{
    CHECK_STACK_HASH_RETURN_ERROR(stk);

    CHECK_CONDITION_RETURN_ERROR(stk == NULL,     STRUCT_NULL_ERROR);
    CHECK_CONDITION_RETURN_ERROR(stk->data == NULL, DATA_NULL_ERROR);

    CHECK_DATA_HASH_RETURN_ERROR(stk);

    stk->capacity = (int)((float)stk->capacity * coef);

    #ifdef CANARY_PROTECT

    // Move the data to the originally allocated place.
    stk->data = (elem_t*)(size_t(stk->data) - sizeof(canary_t));

    elem_t* temp = (elem_t*)realloc(stk->data, stk->capacity * sizeof(elem_t) + 2 * sizeof(canary_t));
    if (temp == NULL)
    {
        stk->capacity = (int)((float)stk->capacity / coef);
        return MEMORY_ALLOCATION_ERROR;
    }

    stk->data = temp;

    ((canary_t*)stk->data)[0] = CANARY_VALUE;

    // Move the data pointer to the real data.
    stk->data = (elem_t*)((size_t)stk->data + sizeof(canary_t));
    ((canary_t*)(stk->data + stk->capacity))[0] = CANARY_VALUE;

    #else

    elem_t* temp = (elem_t*)realloc(stk->data, sizeof(elem_t) * stk->capacity);
    if (temp == NULL) return MEMORY_ALLOCATION_ERROR;
     
    stk->data = temp;

    #endif


    #ifndef RELEASE

    for (int i = (int)((float)stk->capacity / coef); i < stk->capacity; i++)
    {
        stk->data[i] = POISON;
    }

    #endif

    return NO_ERROR;

}


StackError stackInit_internal(Stack* stk, size_t capacity, StackInitInfo info)
{
    CHECK_CONDITION_RETURN_ERROR(stk == NULL, DATA_NULL_ERROR);

    stk->capacity = (int)capacity;
    stk->size = 0;
    stk->info = info;

    #ifdef CANARY_PROTECT
    stk->leftCanary  = CANARY_VALUE;
    stk->rightCanary = CANARY_VALUE;
    #endif

    #ifdef CANARY_PROTECT
    
    // Allocate memory for data and 2 canary elements.
    stk->data = (elem_t*)malloc(capacity * sizeof(elem_t) + 2 * sizeof(canary_t));
    CHECK_CONDITION_RETURN_ERROR(stk->data == NULL, MEMORY_ALLOCATION_ERROR);

    // Set the left canary.
    ((canary_t*)stk->data)[0] = CANARY_VALUE;

    // Move the data pointer to the real data.
    stk->data = (elem_t*)((size_t)stk->data + sizeof(canary_t));

    // Set the right canary.
    ((canary_t*)(stk->data + stk->capacity))[0] = CANARY_VALUE;

    #else
    stk->data = (elem_t*)malloc(capacity * sizeof(elem_t));
    CHECK_CONDITION_RETURN_ERROR(stk->data == NULL, MEMORY_ALLOCATION_ERROR);
    #endif


    #ifndef RELEASE
    for (int i = 0; i < stk->capacity; i++)
        stk->data[i] = POISON;
    #endif

    UPDATE_HASH(stk);

    return NO_ERROR;
}


StackError stackInit_internal(Stack* stk, StackInitInfo info)
{
    return stackInit_internal(stk, STACK_SIZE_DEFAULT, info);
}


StackError stackPush(Stack* stk, const elem_t elem)
{
    CHECK_CONDITION_RETURN_ERROR(stk == NULL, STRUCT_NULL_ERROR);

    CHECK_STACK_HASH_RETURN_ERROR(stk);

    CHECK_DUMP_AND_RETURN_ERROR(stk);

    CHECK_DATA_HASH_RETURN_ERROR(stk);    
    
    if (stk->size >= stk->capacity)
    {
        StackError error = increaseCapacity(stk, STACK_CAPACITY_MULTIPLIER);
        DUMP_AND_RETURN_ERROR(stk, error);
    }

    stk->data[stk->size++] = elem;

    UPDATE_HASH(stk);
    return NO_ERROR;
}


StackError stackPop(Stack* stk, elem_t* elem)
{
    CHECK_CONDITION_RETURN_ERROR(stk == NULL, ELEM_NULL_ERROR);
    CHECK_CONDITION_RETURN_ERROR(elem == NULL, ELEM_NULL_ERROR);
    CHECK_CONDITION_RETURN_ERROR(stk->data == NULL, DATA_NULL_ERROR);
    CHECK_CONDITION_RETURN_ERROR(stk->size <= 0, POP_OUT_OF_RANGE_ERROR); 

    CHECK_STACK_HASH_RETURN_ERROR(stk);
    
    CHECK_DUMP_AND_RETURN_ERROR(stk);
    
    CHECK_DATA_HASH_RETURN_ERROR(stk);

    // If the size is STACK_CAPACITY_MULTIPLIER^2 smaller, than the capacity,...
    if (stk->size <= (int)((float)stk->capacity / (2.0 * STACK_CAPACITY_MULTIPLIER)) 
        && stk->size >= STACK_SIZE_DEFAULT)
        increaseCapacity(stk, 1.0/STACK_CAPACITY_MULTIPLIER); //... decrease the capacity.

    #ifndef RELEASE
    stk->data[stk->size] = POISON;
    #endif
    *elem = stk->data[--stk->size];
    
    UPDATE_HASH(stk);
    return NO_ERROR;
}

inline static void freeData(Stack* stk)
{
    #ifdef CANARY_PROTECT
        free((char*)stk->data - sizeof(canary_t));
    #else
        free((char*)stk->data);
    #endif
}

StackError stackDtor(Stack* stk)
{
    CHECK_CONDITION_RETURN_ERROR(stk       == NULL, STRUCT_NULL_ERROR);
    CHECK_CONDITION_RETURN_ERROR(stk->data == NULL,   DATA_NULL_ERROR);

    CHECK_STACK_HASH_RETURN_ERROR(stk);
    
    CHECK_DUMP_AND_RETURN_ERROR(stk);

    freeData(stk);

    CHECK_DATA_HASH_RETURN_ERROR(stk);

    #ifndef RELEASE

    for (int i = 0; i < stk->capacity; i++)
    {
        stk->data[i] = POISON;
    }

    #endif

    if (stkerr != stderr)
    {
        if (fclose(stkerr) != 0)
            return CLOSING_FILE_ERROR;
    }

    return NO_ERROR;
}


void stackDump_internal(const Stack* stk, const StackError err,
               const char* fileName, const size_t line, const char* funcName)
{
    #define print(...) fprintf(stkerr, __VA_ARGS__)
    #define printColor(color, str,...) fprintf(stkerr, "<font color=" #color ">" str "</font>", __VA_ARGS__)
    
    print("<pre>");

    if (err == STRUCT_NULL_ERROR)
    {
        printColor(red, "NULL stack in file %s(%lu) function %s\n", fileName, line, funcName);
        return;
    }

    print("----------------------------------------------------------------\n");

    printColor(purple, "%s[%p] ", stk->info.varName, stk);
    print("was initialized in ");
    printColor(purple, "%s ", stk->info.fileName);
    print("function ");
    printColor(purple, "%s(%d)\n", stk->info.funcName, stk->info.lineNum);
    print("\tcalled from ");
    printColor(purple, "%s ", fileName);
    print("function ");
    printColor(purple, "%s(%lu):\n", funcName, line);
    
    if (err == NO_ERROR)
        printColor(green, "\t\t\t  ERROR CODE: %s\n", ErrorString[err]);
    else
        printColor(red, "\t\t\t  ERROR CODE: %s\n", ErrorString[err]);

    #ifdef CANARY_PROTECT
    if (stk->leftCanary == CANARY_VALUE)
        printColor(green ,"\t *leftCanary:" CANARY_FORMAT "\n", stk->leftCanary);
    else
        printColor(red ,"\t *leftCanary:" CANARY_FORMAT "\n", stk->leftCanary);
    #endif

    print("\t *size = %d      \n", stk->size);
    print("\t *capacity = %d  \n", stk->capacity);
    print("\t *data[%p]:      \n", stk->data);
    
    
    if (err != NEGATIVE_CAPACITY_ERROR && err != UNREGISTERED_STRUCT_ACCESS_ERROR && err != SIZE_CAPACITY_ERROR)
    {
        #ifdef CANARY_PROTECT
        canary_t leftCanary = *(canary_t*)((size_t)stk->data - sizeof(canary_t));
        if (leftCanary == CANARY_VALUE)
            printColor(green ,"\t\t *leftCanary:" CANARY_FORMAT "\n", leftCanary);
        else
            printColor(red ,"\t\t *leftCanary:" CANARY_FORMAT "\n", leftCanary);
        #endif
        for (int i = 0; i < stk->capacity; i++)
        {
            // <(int)log10(stk->capacity) + 1> is the amount of digits in a number.
            if (i == stk->size)              printColor("blue", "%s", "\t\t> ");
            else if (stk->data[i] == POISON) print("\t\tO ");
            else                             print("\t\t@ ");

            print("data[%.*d] = ", (int)log10(stk->capacity) + 1, i);
            if (stk->data[i] == POISON) print("POISON");
            else                        print(ELEM_FORMAT, stk->data[i]);

            if (i == stk->size)  printColor("blue", "%s", " <\n");
            else                 print("  \n");
        }
        #ifdef CANARY_PROTECT        
        canary_t rightCanary = *(canary_t*)(stk->data + stk->capacity);
        if (rightCanary == CANARY_VALUE)
            printColor(green ,"\t\t *rightCanary:" CANARY_FORMAT "\n", rightCanary);
        else
            printColor(red ,"\t\t *rightCanary:" CANARY_FORMAT "\n", rightCanary);
        #endif
   }
    #ifdef CANARY_PROTECT
    if (stk->rightCanary == CANARY_VALUE)
        printColor(green ,"\t *rightCanary:" CANARY_FORMAT "\n", stk->rightCanary);
    else
        printColor(red ,"\t *rightCanary:" CANARY_FORMAT "\n", stk->rightCanary);
    #endif

    #undef print
    #undef print_color
}


static unsigned long long calculateHash(const char* dataStart, const size_t size)
{
    assert(dataStart);
    unsigned long long hash = +79653421411ull; 
    for (size_t i = 0; i < size; i++)
    {
        hash += int(dataStart[i]) * i;
    }
    return hash;
}


static unsigned long long calculateStackHash(const Stack* stk)
{
    assert(stk);
    unsigned long long hash = 0ull;

    size_t stackSize = sizeof(&(stk->data)) + sizeof(stk->size) + sizeof(stk->capacity);
    hash += calculateHash((const char*)(&(stk->data)), stackSize);

    return hash;
}


static unsigned long long calculateDataHash(const Stack* stk)
{
    assert(stk);
    unsigned long long hash = 0ull;

    hash += calculateHash((const char*)stk->data, stk->size * sizeof(elem_t));

    return hash;
}


StackError setLogFile(const char* fileName)
{
    FILE* file = fopen(fileName, "w");
    if (file == NULL)
        return OPENING_FILE_ERROR;

    stkerr = file;

    return NO_ERROR;
}
