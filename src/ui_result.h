/*
    This file is part of esp-thermostat.

    esp-thermostat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-thermostat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-thermostat.  If not, see <http://www.gnu.org/licenses/>.
    
    Author: Tamas Karpati
    Created on 2017-01-08
*/

#ifndef UI_RESULT_H
#define	UI_RESULT_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {
	UI_RESULT_IDLE,
	UI_RESULT_UPDATE,
	UI_RESULT_SWITCH_MAIN_SCREEN,
	UI_RESULT_SWITCH_MENU_SCREEN,
	UI_RESULT_SWITCH_SCHEDULING_SCREEN
} ui_result;

#ifdef	__cplusplus
}
#endif

#endif	/* UI_RESULT_H */

