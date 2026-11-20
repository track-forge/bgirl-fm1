// SPDX-License-Identifier: GPL-3.0-only
// Disposable in-memory instrument: deliberately no storage, MIDI or microphone.
const start=document.querySelector('#start'),answer=document.querySelector('#answer'),stop=document.querySelector('#stop'),status=document.querySelector('#status');
let ac,node,loading=false,running=false,serial=0;
const pending=new Map();
function request(action){
 const id=++serial;
 return new Promise((resolve,reject)=>{
  const timer=setTimeout(()=>{pending.delete(id);reject(new Error('Instrument timed out'));},15000);
  pending.set(id,{resolve,reject,timer});node.port.postMessage({type:'flyer',action,id});
 });
}
async function boot(){
 ac=new AudioContext({sampleRate:44100,latencyHint:'interactive'});
 const resumed=ac.resume(); // Request audio while the button's user activation is live.
 const [wasm]=await Promise.all([fetch('felucca.wasm').then(r=>{if(!r.ok)throw Error('Instrument unavailable');return r.arrayBuffer();}),ac.audioWorklet.addModule('worklet.js')]);
 node=new AudioWorkletNode(ac,'felucca',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[2]});node.connect(ac.destination);
 await new Promise((resolve,reject)=>{
  const timer=setTimeout(()=>reject(Error('Instrument timed out')),15000);
  node.port.onmessage=({data:m})=>{
   if(m.type==='ready'){clearTimeout(timer);resolve();}
   if(m.type==='error'){clearTimeout(timer);reject(Error(m.message));}
   if(m.type==='flyer-result'){
    const p=pending.get(m.id);if(p){clearTimeout(p.timer);pending.delete(m.id);m.ok?p.resolve():p.reject(Error('Instrument could not play'));}
   }
   // Frame/flash messages intentionally discarded: never persist flyer music.
  };
  node.port.postMessage({type:'load',wasm,sectors:{},master:650},[wasm]);
 });
 await resumed;
}
async function fail(err){
 running=false;answer.disabled=stop.disabled=true;start.disabled=false;
 if(ac)await ac.close().catch(()=>{});ac=node=null;
 status.textContent='Audio could not start. Try again, or open the full studio. '+err.message;
}
start.onclick=async()=>{
 if(loading)return;loading=true;start.disabled=true;status.textContent='Starting the beat…';
 try{
  if(!node)await boot();else await ac.resume();
  await request(0);running=true;answer.disabled=stop.disabled=false;
  status.textContent='The beat is yours. Tap Answer whenever you want the voice to drop.';
 }catch(e){await fail(e);}finally{loading=false;}
};
answer.onclick=async()=>{
 if(!running)return;
 try{await request(1);status.textContent='Your answer. Try another placement in the loop.';}catch(e){await fail(e);}
};
stop.onclick=async()=>{
 if(!running)return;
 running=false;answer.disabled=stop.disabled=true;
 try{await request(2);await ac.suspend();start.disabled=false;status.textContent='Stopped. Start again to try another answer.';}catch(e){await fail(e);}
};
document.addEventListener('visibilitychange',()=>{if(document.hidden&&running)stop.click();});
