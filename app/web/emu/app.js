/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
// Loaded by the pinned platform page after user-initiated switch-on.
export function attach() {
  const header=document.querySelector('header');
  const intro=document.createElement('p');
  intro.textContent='ELECTRO / 01 — machine drums · mono bass · experimental robot voice';
  header.appendChild(intro);
  const guide=document.createElement('section');
  guide.setAttribute('aria-label','BGIRL performance controls');
  const heading=document.createElement('h2'); heading.textContent='Rock the beat';
  const text=document.createElement('p');
  text.textContent='ARP starts/stops the groove. KNOB 1: 60–180 BPM. KNOB 2: bass tone. FX mutes/returns drums without losing the beat. PRESETS switches BASS / experimental VOICE; keys play the selected part. MASTER sets volume; HOME stops and panics. Arrow keys turn focused knobs; hold Space to play a focused key.';
  const status=document.createElement('p');
  status.textContent='Local development slice. Voice needs listening approval. No device build or hardware validation. SAVE stores octave, part and Bluetooth only—not tempo, tone, mute or transport.';
  guide.append(heading,text,status);
  document.querySelector('main').prepend(guide);
  document.querySelector('#mute').textContent='Mute all audio';
  const descriptions={ARP:'Start or stop groove; restart on step 1',FX:'Mute or return drums, clock continues',PRESETS:'Select bass or experimental voice', 'KNOB 1':'Tempo, 60 to 180 beats per minute','KNOB 2':'Bass tone, dark to bright',HOME:'Panic: silence notes and stop groove'};
  for(const el of document.querySelectorAll('.hit')) {
    const description=descriptions[el.getAttribute('aria-label')];
    if(description){el.setAttribute('aria-description',description);el.title=description;}
  }
}
