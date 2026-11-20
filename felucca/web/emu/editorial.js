// SPDX-License-Identifier: GPL-3.0-only
const player=new Audio(),status=document.querySelector('#excerpt-status'),stop=document.querySelector('#stop-excerpt');
let selected=null,serial=0;
function clear(){serial++;player.pause();player.currentTime=0;if(selected)selected.setAttribute('aria-pressed','false');selected=null;stop.disabled=true;}
stop.onclick=()=>{clear();status.textContent='Excerpt stopped.';};
document.querySelectorAll('[data-clip]').forEach(button=>{
 button.setAttribute('aria-pressed','false');
 button.onclick=async()=>{
  if(document.querySelector('#start').disabled&&document.querySelector('#stop').disabled){status.textContent='Wait for the live instrument to finish starting, then choose an excerpt.';return;}
  clear();
  // A single sound source at a time, including the live flyer.
  document.querySelector('#stop').click();
  const token=serial;selected=button;button.setAttribute('aria-pressed','true');
  player.src='editorial/'+button.dataset.clip+'.wav';stop.disabled=false;
  try{await player.play();if(token===serial)status.textContent='Playing: '+button.textContent;}
  catch{if(token===serial){clear();status.textContent='Audio unavailable. Try again or open the full studio.';}}
 };
});
player.onended=()=>{clear();status.textContent='Finished. Choose another color or placement.';};
player.onerror=()=>{clear();status.textContent='Excerpt unavailable. Open the full studio to explore.';};
document.querySelector('#start').addEventListener('click',()=>{clear();status.textContent='Live flyer selected.';});
document.addEventListener('visibilitychange',()=>{if(document.hidden)clear();});
