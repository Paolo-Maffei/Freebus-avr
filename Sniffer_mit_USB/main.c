/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2009 Thomas Weyhrauch <thomas@weyhrauch.de>
 *
 *  This program is free software; you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software Foundation; either version 3 of the License.
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program;
 *  if not, see <http://www.gnu.org/licenses/>.
 *
 *
 *
 *  Parts of this software are baed on the work of:
 *  S. Seegel, post@seegel-systeme.de
 *  Peter Fleury pfleury@gmx.ch
 *
 */


#include "main.h"

int main (void)
{
	//Disable watchdog, because the watchdog timer remains active even after a system reset!!
	MCUSR = 0;
	wdt_disable();
	
	//Initialize the application
	theApp.Init ();


	//Do the mainloop
	while (theApp.bRunApplication)
	{
		CTask::Run();
		wdt_reset ();
	}

	//Shutdown
	cli ();
	theApp.OnShutdown ();
	wdt_disable();
	return 0;
}
