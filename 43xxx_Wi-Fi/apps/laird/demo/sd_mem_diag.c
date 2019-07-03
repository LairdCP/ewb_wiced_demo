/***************************************************************************************************

 File:     sd_mem_diag.c
 Author:   Laird Technologies
 Version:  0.1

 Description:

The MIT License (MIT)

Copyright 2016, Laird Technologies (Laird)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sub license, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

***************************************************************************************************/


#include <stdlib.h>
#include <stdint.h>

#include "wiced.h"
#include "sd_common_defs.h"

#include "sd_mem_diag.h"

/******************************************************************************
* Constants
******************************************************************************/

// 256 block of 256 bytes is 64 k
#define MALLOC_BLOCK_SIZE      256 // size of each malloc block
#define MALLOC_BLOCK_NUM_MAX   512 // maxiumum number of blocks we can malloc

#define STACK_CHK_ARRAY_SIZE   16  // maximum number of tasks that can register to have their stack checked

#define STACK_FILL_WORD        0xEFEFEFEF // ThreadX stack fill pattern
#define STACK_CHECK_RESOLUTION 64 // check remaining stack in increments of 64 bytes

/******************************************************************************
* Type definitions
******************************************************************************/

// Struct definition for a check of the ThreadX remaining stack
typedef struct
{
    const TX_THREAD *pThread;    // point to the ThreadX task control struct
    uint32_t *highWaterStackPtr; // closest point to tx_thread_stack_start we ahve seen
} stackCheckInfo_t;

/******************************************************************************
* Local varaibles
******************************************************************************/

static uint32_t * memBlocks[MALLOC_BLOCK_NUM_MAX];

// pointer to const since it is the stack control struct
static stackCheckInfo_t stackCheckInfoArray[STACK_CHK_ARRAY_SIZE];

/******************************************************************************
* Local functions
******************************************************************************/

/*******************************************************************************
*
* @brief clears the mem pointer array we use to track what we malloc'd
*
* @param none
*
*
* @return WICED_SUCCESS
*****************************************************************************/
static void clear_all_malloc_blocks (void)
{
    int i;
    for (i = 0; i < MALLOC_BLOCK_NUM_MAX; i++)
    {
        memBlocks[i] = NULL;
    }
}

/*******************************************************************************
*
* @brief  malloc as man blocks as we can or until the pointer array is full
*
* @param  none
*
*
* @return WICED_SUCCESS
*****************************************************************************/
static int32_t malloc_n_blocks (void)
{
    static uint32_t *pMem; // static to keep it visible in the debugger
    int i;

    for(i = 0; i < MALLOC_BLOCK_NUM_MAX; i++)
    {
        pMem = (uint32_t *)malloc(MALLOC_BLOCK_SIZE);
        if (pMem == NULL)
        {
            break;
        }

        memBlocks[i] = pMem;
    }

    return i * MALLOC_BLOCK_SIZE;
}

/*******************************************************************************
*
* @brief   free all the blocks refered to by the mem pointer array
*
* @param  none
*
*
* @return WICED_SUCCESS
*****************************************************************************/
static void free_n_blocks (void)
{
    int i;

    for (i = 0; i < MALLOC_BLOCK_NUM_MAX; i++)
    {
        if(memBlocks[i] != NULL)
        {
            free(memBlocks[i]);
        }
    }
}


/******************************************************************************
 *
 * @brief   Checks for the amount of unused stack in a ThreadX task control struct
 *            TX_THREAD.  The search for unused stack increments by the stack
 *            check resolution to make it faster.
 *
 * @note    This function saves the high water mark to speed up the next search
 *
 * @param  pStackInfo pointer to our struct definition that keeps stack info
 *
 * @return WICED_SUCCESS
 ******************************************************************************/
wiced_result_t PrintTaskStackInfo (stackCheckInfo_t * pStackInfo)
{
    int32_t minStackRemaining = 0;
    const TX_THREAD* pThread = pStackInfo->pThread;  // const to make sure we don't trash the task control block

    // start at out last high water mark
    uint32_t *pCheckLocation = pStackInfo->highWaterStackPtr;

    // loop until we find unused stack or we hit the end
    while((uint32_t)pCheckLocation >
          (uint32_t)(pThread->tx_thread_stack_start))
    {
        // If we find two consecutive unused location assume we have found unused stack
        if (*pCheckLocation == STACK_FILL_WORD)
        {
            if(*(pCheckLocation - 1) == STACK_FILL_WORD)
            {
                // If we have found unused stack, save the high water mark so we don't have as
                // much to search next time if it has moved back toward the end (initial stack ptr)
                pStackInfo->highWaterStackPtr = pCheckLocation;
                break;
            }
        }

        // move in N byte steps we don't need the exact number of available bytes since we shouldn't cut it close...
        pCheckLocation -= STACK_CHECK_RESOLUTION;
    }

    // We have found an open location
    minStackRemaining = (uint32_t)pCheckLocation - (uint32_t)pThread->tx_thread_stack_start;

    // we print from inside the function so we don't have to expose pThread....
    WPRINT_APP_INFO(("Stack: %-20s > %d\n", pThread->tx_thread_name, (int)minStackRemaining));

    return WICED_SUCCESS;
}


/******************************************************************************
 *
 * @brief  Call this funciton periodically to determine how much stack space is remaining
 *
 * @note  Checks one task stack per call to avoid using too much CPU all at once
 *
 * @param  none
 *
 * @return WICED_SUCCESS
 ******************************************************************************/
wiced_result_t CheckNextThreadStack (void)
{
    wiced_result_t status = WICED_SUCCESS;
    static int32_t stackIndex = 0;            // saved across calls to move through task stack array

    // if the stack info array entry is non-null we have a task that registered for monitoring
    if(stackCheckInfoArray[stackIndex].pThread != NULL)
    {
        PrintTaskStackInfo(&(stackCheckInfoArray[stackIndex]));
        stackIndex++;  // increment to the next task stack to check
    }
    else
    {
        // if we hit the end of the array, start back at the beginning
        stackIndex = 0;
    }

    // make stack index modulo N
    if(stackIndex >= STACK_CHK_ARRAY_SIZE)
    {
        stackIndex = 0;
    }

    return status;
}

/******************************************************************************
* Global functions
******************************************************************************/

/******************************************************************************
 *
 * @brief  Call this funciton periodically to determine how much stack space is remaining
 *
 * @note
 *
 * @param  none
 *
 * @return WICED_SUCCESS
 ******************************************************************************/
wiced_result_t sd_checkThreadStacks( void )
{
    return CheckNextThreadStack();
}

/******************************************************************************
 *
 * @brief  Allows tasks to register to have their remaining stack space checked
 *            periodically
 *
 * @note   >>> THREADX ONLY  <<<
 *         ADD A CALL TO THIS FUNCTION TO THE LAST LINE OF
 *         wiced_rtos_create_thread( wiced_thread_t* thread, uint8_t priority, const char* name,...
 *
 *         located in the file  WICED/RTOS/ThreadX/WICED/wiced_rtos.c
 *
 *         pass it the thread param which is passed into the wiced_rtos_create_thread()
 *
 * @param  pThread pointer to a wiced thread definition struct
 *
 * @return WICED_SUCCESS or ERROR
 ******************************************************************************/
wiced_result_t sd_RegisterThreadWithStackMonitor (wiced_thread_t *pThread)
{
    int i;

    /* Look for an empty spot for the thread */
    for (i = 0; i < STACK_CHK_ARRAY_SIZE; i++)
    {
        if(stackCheckInfoArray[i].pThread == NULL)
        {
            /*
             * WICED has a generic thread pointer but this code depends
             * on threadx since we need access to specific struct members
             */
            stackCheckInfoArray[i].pThread = (TX_THREAD*)pThread;

            /*
             * Use the current stack pointer as the high water mark as
             * the starting point
             */
            stackCheckInfoArray[i].highWaterStackPtr =
                ((TX_THREAD*)pThread)->tx_thread_stack_ptr;

            return WICED_SUCCESS;
        }
    }

    /* Couldn't find an empty slot */
    return WICED_ERROR;
}


/*******************************************************************************
*
* @brief  checks the available c heap memory by allocating as much as it can
*           and then freeing it
*
* @param  none
*
*
* @return int number of bytes available
*****************************************************************************/
int sd_check_avail_mem (void)
{
    int total;

    // clear the array we use to keep track of the malloc'd blocks
    clear_all_malloc_blocks();

    // malloc as many blocks as we can
    total = malloc_n_blocks();

    // free all the blocks we malloc'd
    free_n_blocks();

    return total;
}

