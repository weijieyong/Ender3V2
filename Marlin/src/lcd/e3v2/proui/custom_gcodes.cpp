/**
 * Custom G-code implementation for PRO UI
 * Author: Miguel A. Risco-Castillo (MRISCOC)
 * Version: 2.2.0
 * Date: 2023/08/04
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../../inc/MarlinConfigPre.h"

#if ALL(DWIN_LCD_PROUI, HAS_CGCODE)

#include "../../../MarlinCore.h" // for wait_for_user
#include "../../../core/types.h"
#include "../../../gcode/gcode.h"
#include "../../../libs/buzzer.h"
#include "../../marlinui.h"

#include "dwin.h"
#include "custom_gcodes.h"

#if ALL(PROUI_EX, HAS_MEDIA)
  #include "file_header.h"
#endif

#if ENABLED(LCD_BED_TRAMMING)
  #include "bed_tramming.h"
#endif

//=============================================================================
// Extended G-CODES
//=============================================================================

void cError() {
  parser.unknown_command_warning();
}

#if HAS_CUSTOM_COLORS
// C11 Set color for UI element E
  void C11() {
    const int16_t E = parser.seenval('E') ? parser.value_byte() : 0; // UI element
    if (E) {
      hmiValue.Color[0] = parser.seenval('R') ? parser.value_byte() : 0;
      hmiValue.Color[1] = parser.seenval('G') ? parser.value_byte() : 0;
      hmiValue.Color[2] = parser.seenval('B') ? parser.value_byte() : 0;
      dwinApplyColor(E);
    } else dwinRedrawScreen();
  }
#endif

#if ENABLED(LCD_BED_TRAMMING)
  // Bed tramming
  void C35() {
    if (parser.seenval('T')) {
      const int8_t i = parser.value_byte();
      if (WITHIN(i, 0, 4)) tram(i);
    }
    TERN_(HAS_BED_PROBE, else trammingwizard());
  }
#endif

// Cancel a Wait for User without an Emergecy Parser
void C108() {
  #if DEBUG_DWIN
    SERIAL_ECHOLNPGM(F("wait_for_user was "), wait_for_user);
    SERIAL_ECHOLNPGM(F("checkkey was "), checkkey);
  #endif
  #if LCD_BACKLIGHT_TIMEOUT_MINS
    ui.refresh_backlight_timeout();
  #endif
  if (!ui.backlight) ui.refresh_brightness();
  wait_for_user = false;
  DONE_BUZZ(true);
}

// Enable or disable preview screen
#if ENABLED(HAS_GCODE_PREVIEW)
void C250() {
  if (parser.seenval('P')) {
    hmiData.enablePreview = !!parser.value_byte();
  }
  SERIAL_ECHOLNPGM(F("PREVIEW:"), hmiData.enablePreview);
}
#endif

// lock/unlock screen
#if HAS_LOCKSCREEN
  void C510() {
    if (parser.seenval('U') && parser.value_int()) dwinUnLockScreen();
    else dwinLockScreen();
  }
#endif

#if DEBUG_DWIN
  #include "../../../module/planner.h"
  void C997() {
    dwinRebootScreen();
    SERIAL_ECHOLNPGM("Simulating a printer freeze");
    while (1) {};
  }
#endif

// Special Creality DWIN GCodes
void customGcode(const int16_t codenum) {
  switch(codenum) {
    #if HAS_CUSTOM_COLORS
      case 11: C11(); break;            // Set color for UI element E
    #endif
    #if ENABLED(LCD_BED_TRAMMING)
      case 35: C35(); break; // Launch bed tramming wizard
    #endif
    case 108: C108(); break;            // Cancel a Wait for User without an Emergecy Parser
    #if ENABLED(HAS_GCODE_PREVIEW)
      case 250: C250(); break;          // Enable or disable preview screen
    #endif
    #if HAS_LOCKSCREEN
      case 510: C510(); break;          // lock screen
    #endif
    #if DEBUG_DWIN
      case 997: C997(); break;          // Simulate a printer freeze
    #endif
    #if PROUI_EX
      #if HAS_MEDIA
        case 10: proUIEx.C10(); break;          // Mark the G-code file as a Configuration file
      #endif
      #if HAS_MESH
        case 29: proUIEx.C29(); break;    // Set probing area and mesh leveling settings
      #endif
      case 100: proUIEx.C100(); break;    // Change Physical minimums
      case 101: proUIEx.C101(); break;    // Change Physical maximums
      case 102: proUIEx.C102(); break;    // Change Bed size
      case 104: proUIEx.C104(); break;    // Set extruder max temperature (limited by maxtemp in thermistor table)
      case 115: proUIEx.C115(); break;    // ProUI Info
      #if ENABLED(NOZZLE_PARK_FEATURE)
        case 125: proUIEx.C125(); break;  // Set park position
      #endif
      #if HAS_PROUI_RUNOUT_SENSOR
        case 412: proUIEx.C412(); break;  // Set runout sensor active mode
      #endif
      case 562: proUIEx.C562(); break;    // Invert Extruder
      case 851: proUIEx.C851(); break;    // If has a probe set z feed rate and multiprobe, if not, set manual z-offset
      #if HAS_TOOLBAR
        case 810: proUIEx.C810(); break;  // Config toolbar
      #endif
    #endif
    default: cError(); break;
  }
}

void customGcodeReport(const bool forReplay/*=true*/) {
  #if PROUI_EX
    proUIEx.C100_report(forReplay);
    proUIEx.C101_report(forReplay);
    proUIEx.C102_report(forReplay);
    #if HAS_MESH
      proUIEx.C29_report(forReplay);
    #endif
    proUIEx.C104_report(forReplay);
    #if ENABLED(NOZZLE_PARK_FEATURE)
      proUIEx.C125_report(forReplay);
    #endif
    #if HAS_PROUI_RUNOUT_SENSOR
      proUIEx.C412_report(forReplay);
    #endif
      proUIEx.C562_report(forReplay);
    #if HAS_BED_PROBE
      proUIEx.C851_report(forReplay);
    #endif
  #endif
}

#endif // DWIN_LCD_PROUI && HAS_CGCODE
