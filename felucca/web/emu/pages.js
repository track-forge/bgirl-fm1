// SPDX-License-Identifier: GPL-3.0-only
// Reveal adapted from Hortator web/sim/layout.js, 2026 DEADACTIVE.
const device = document.querySelector('#device');
const lcd = document.querySelector('.lcd');
const preview = document.querySelector('#preview');
const screen = document.querySelector('#screen');
const power = document.querySelector('#power-on');
const patch = document.querySelector('#patch');
const status = document.querySelector('#note');
const skip = document.querySelector('#skip');
const keys = document.querySelector('#keys');
const motion = matchMedia('(prefers-reduced-motion: reduce)');
const bindings = 'awsedftgyhuj';
const names = ['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'];
let audio, node, held, reveal;
let whites = 0;

function settle() {
  reveal?.finish();
  skip.hidden = true;
}

// Unlike Hortator's fly-in, BGIRL starts at the LCD's position and pulls back.
// Transform the whole panel so the dancer never leaves the LCD.
if (!motion.matches) {
  const panel = device.getBoundingClientRect();
  const display = lcd.getBoundingClientRect();
  const origin = display.left + display.width / 2 - panel.left;
  device.style.transformOrigin = `${origin}px ${display.top + display.height / 2 - panel.top}px`;
  const offset = panel.width / 2 - origin;
  reveal = device.animate([
    {transform:`translateX(${offset}px) scale(2.65)`, offset:0},
    {transform:`translateX(${offset}px) scale(2.65)`, offset:.28},
    {transform:'none', offset:1}
  ], {duration:2450,easing:'cubic-bezier(.32,0,.2,1)'});
  reveal.onfinish = () => { skip.hidden = true; };
} else {
  preview.src = 'crew-home.png';
  skip.hidden = true;
}
skip.addEventListener('click', () => { settle(); power.focus(); });
motion.addEventListener('change', () => {
  if (motion.matches) settle();
  preview.src = motion.matches ? 'crew-home.png' : 'crew-home.webp';
});

const ctx = screen.getContext('2d');
const pixels = ctx.createImageData(240,240);
function drawScreen(fb) {
  for (let i=0; i<fb.length; i++) {
    const v = ((fb[i] >> 8) | (fb[i] << 8)) & 0xffff;
    pixels.data[i*4] = ((v >> 11) & 31) * 255 / 31;
    pixels.data[i*4+1] = ((v >> 5) & 63) * 255 / 63;
    pixels.data[i*4+2] = (v & 31) * 255 / 31;
    pixels.data[i*4+3] = 255;
  }
  ctx.putImageData(pixels,0,0);
}
const wasm = fetch('felucca.wasm').then(response => {
  if (!response.ok) throw new Error('Instrument unavailable');
  return response.arrayBuffer();
});
wasm.catch(() => {}); // The power gesture reports a failed preload.

function start() {
  if (audio) return;
  settle();
  power.disabled = true;
  status.textContent = 'Starting instrument…';
  void boot();
}
async function boot() {
  let candidate;
  let timeout;
  try {
    candidate = new AudioContext({sampleRate:44100});
    audio = candidate;
    await candidate.resume();
    await candidate.audioWorklet.addModule('worklet.js');
    const worklet = new AudioWorkletNode(candidate,'felucca',{numberOfInputs:0,outputChannelCount:[2]});
    const binary = (await wasm).slice(0);
    await new Promise((resolve,reject) => {
      timeout = setTimeout(() => reject(new Error('Instrument startup timed out')),5000);
      worklet.onprocessorerror = () => reject(new Error('Audio processor failed'));
      worklet.port.onmessage = ({data}) => {
        if (data.type === 'error') reject(new Error(data.message));
        if (data.type === 'ready') resolve();
        if (data.fb) {
          drawScreen(data.fb);
          screen.hidden = false;
          preview.hidden = true;
        }
      };
      worklet.port.postMessage({type:'load',pages:true,wasm:binary,master:650},[binary]);
      worklet.connect(candidate.destination);
    });
    node = worklet;
    node.port.postMessage({type:'pages-patch',trio:Number(patch.value)});
    if (held) node.port.postMessage({type:'pages-note',note:held.note});
    power.textContent = 'Sound enabled';
    status.textContent = 'Vocoder ready · T4 voice → T3 carrier';
  } catch {
    if (candidate) await candidate.close().catch(() => {});
    audio = undefined;
    power.disabled = false;
    status.textContent = 'Audio could not start. Retry or open the full studio.';
  } finally {
    clearTimeout(timeout);
  }
}

function press(note, source, button) {
  release();
  held = {note,source,button};
  button.classList.add('held');
  if (node) node.port.postMessage({type:'pages-note',note});
  else start();
}
function release(source) {
  if (!held || (source !== undefined && held.source !== source)) return;
  held.button.classList.remove('held');
  held = undefined;
  node?.port.postMessage({type:'pages-note',note:0});
}

names.forEach((name,index) => {
  const button = document.createElement('button');
  button.type = 'button';
  button.className = 'key';
  button.textContent = name;
  button.setAttribute('aria-label',`${name} · ${bindings[index].toUpperCase()}`);
  if (name.includes('♯')) {
    button.classList.add('sharp');
    button.style.left = `${whites / 7 * 100}%`;
  } else whites++;
  button.addEventListener('pointerdown', event => {
    if (event.button !== 0) return;
    button.setPointerCapture(event.pointerId);
    press(48+index,event.pointerId,button);
  });
  for (const type of ['pointerup','pointercancel','lostpointercapture']) {
    button.addEventListener(type,event => release(event.pointerId));
  }
  button.addEventListener('keydown', event => {
    if (!['Enter',' '].includes(event.key)) return;
    event.preventDefault();
    if (!event.repeat) press(48+index,event.code,button);
  });
  keys.append(button);
});
power.addEventListener('click',start);
patch.addEventListener('change',() => {
  release();
  node?.port.postMessage({type:'pages-patch',trio:Number(patch.value)});
});
addEventListener('keydown',event => {
  const index = bindings.indexOf(event.key.toLowerCase());
  if (index<0 || event.repeat || event.ctrlKey || event.metaKey || event.altKey || event.target.closest?.('select,input,textarea')) return;
  press(48+index,event.code,keys.children[index]);
});
addEventListener('keyup',event => release(event.code));
addEventListener('blur',() => release());
document.addEventListener('visibilitychange',() => { if (document.hidden) release(); });
document.querySelector('a[href="#keys"]').addEventListener('click',event => {
  event.preventDefault();
  settle();
  keys.firstElementChild.focus();
});
