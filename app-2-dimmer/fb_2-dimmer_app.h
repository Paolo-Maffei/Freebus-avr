/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
/**
* @file   fb_2-dimmer_app.h
* @author 
* @date   Sun Aug 24 21:35:14 2008
* 
* @brief  
* 
* 
*/

#ifndef _FB_2_DIMMER_APP_H
#define _FB_2_DIMMER_APP_H

#include "msg_queue.h"

uint8_t restartApplication(void);
uint8_t runApplication(struct msg *rxmsg);
uint8_t readApplication(struct msg *rxmsg);
uint8_t restartApplication(void);
void setApplicationDefaults(void);


#endif /* _FB_2_DIMMER_APP_H */
