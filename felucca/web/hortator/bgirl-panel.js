// SPDX-License-Identifier: GPL-3.0-only
import { BTN, ENC } from './controls.js';

// Browser bridge roles in felucca_web.c / the studio panel.
const buttons = ['B_FX','B_SCL','B_ENV','B_LFO','B_EDIT','B_GLO','B_HOME','B_SAVE',
  'B_ARP','B_SEQ','B_PLAY','B_REC','B_OCTDN','B_OCTUP'];
const encoders = ['EN_SELECT','EN_ALGO','EN_PRESET','EN_K1','EN_K2','EN_K3','EN_K4'];
const buttonBits = BTN.map(control => buttons.indexOf(control.c));
const encoderRoles = ENC.map(control => encoders.indexOf(control.c));

export function panelInput(port) {
  let buttons = 0, keys = 0;
  return m => {
    if (m.t === 'btn') {
      const bit = 1 << buttonBits[m.id];
      buttons = m.down ? buttons | bit : buttons & ~bit;
      port.postMessage({type:'buttons',mask:buttons});
    } else if (m.t === 'key') {
      // Both panels number their 27 keys chromatically from F3.
      const bit = 1 << m.id;
      keys = m.down ? keys | bit : keys & ~bit;
      port.postMessage({type:'keys',mask:keys});
    } else if (m.t === 'release') {
      buttons = keys = 0;
      port.postMessage({type:'buttons',mask:0});
      port.postMessage({type:'keys',mask:0});
    } else if (m.t === 'enc') {
      port.postMessage({type:'enc',role:encoderRoles[m.id],n:m.steps});
    } else if (m.t === 'master') {
      port.postMessage({type:'master',value:m.value});
    }
  };
}

export function panelLights(leds) {
  return {
    leds:buttonBits.reduce((mask,bit,index) => mask | (((leds[0] | leds[2]) >> bit & 1) << index),0),
    keyLeds:leds[1] | leds[3] | leds[7]
  };
}
