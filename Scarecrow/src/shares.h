/** @file shares.h
 *  This file contains extern declarations of shares and queues which are used
 *  in more than one file of the scarecrow project.
 * 
 *  @author Ethan Nikcevich
 *  @date   11/13/2022
 */

#ifndef _SHARES_H_
#define _SHARES_H_

#include "taskqueue.h"
#include "taskshare.h"

extern Share<bool> cam_share;
extern Share<bool> motors_share;
extern Share<bool> camphoto_share;


// // A share which holds a counter of how many times a simulated event occurred
// extern Share<bool> begin_recording;

// // A queue which triggers a task to print the count at certain times
// extern Queue<uint16_t> data_queue;

#endif // _SHARES_H_

