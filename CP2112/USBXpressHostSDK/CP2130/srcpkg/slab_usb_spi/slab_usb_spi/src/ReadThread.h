/* 
 * File:   readThread.h
 * Author: test
 *
 * Created on August 7, 2014, 7:27 AM
 */

#ifndef READTHREAD_H
#define	READTHREAD_H

extern pthread_mutex_t lock;

void *readThread(void *arg);
int createThread(pthread_t* thread);

#endif	/* READTHREAD_H */

