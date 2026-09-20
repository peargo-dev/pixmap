// ==UserScript==
// @name         hi demidemia
// @namespace    peargo
// @version      1.3
// @description  get rxped
// @author       peargo
// @match        https://pixmap.fun/*
// @match        https://*.pixmap.fun/*
// @run-at       document-start
// @grant        none
// ==/UserScript==
(function(){
  'use strict';

  const origFetch = window.fetch;
  window.fetch = async function(input, init){
    const url = typeof input==='string' ? input : (input&&input.url)||'';
    const res = await origFetch.apply(this, arguments);
    if (/\/api\//i.test(url) || /\/auth/i.test(url) || /\/me/i.test(url)) {
      try{
        const ct = res.headers.get('content-type')||'';
        if (ct.includes('json')) {
          const j = await res.clone().json().catch(()=>null);
          if (j && typeof j==='object') {
            let patched=false;
            const patch = (o)=>{
              if(o && typeof o==='object'){
                if('role' in o){ o.role=200; patched=true; }
                if('userlvl' in o){ o.userlvl=1; patched=true; }
                if('roles' in o){ o.roles=1; patched=true; }
                if(o.user) patch(o.user);
                if(o.data) patch(o.data);
                if(o.currentUser) patch(o.currentUser);
                if(Array.isArray(o)) o.forEach(patch);
                for(const k in o) if(typeof o[k]==='object') patch(o[k]);
              }
            };
            patch(j);
            if(patched) return new Response(JSON.stringify(j), {status:res.status, headers:res.headers});
          }
        }
      }catch(e){}
    }
    return res;
  };

  const open = XMLHttpRequest.prototype.open;
  const send = XMLHttpRequest.prototype.send;
  XMLHttpRequest.prototype.open = function(m,u){ this._url=u; return open.apply(this, arguments); };
  XMLHttpRequest.prototype.send = function(b){
    this.addEventListener('readystatechange', function(){
      if(this.readyState===4 && this.status===200 && this._url && /\/api\//i.test(this._url)){
        try{
          const j=JSON.parse(this.responseText);
          if(j && typeof j==='object' && JSON.stringify(j).includes('"role"')){
            const patched = JSON.parse(this.responseText);
            const patch=(o)=>{ if(o&&typeof o==='object'){ if('role' in o) o.role=200; for(const k in o) if(typeof o[k]==='object') patch(o[k]); }};
            patch(patched);
            Object.defineProperty(this,'responseText',{value:JSON.stringify(patched)});
            Object.defineProperty(this,'response',{value:JSON.stringify(patched)});
          }
        }catch(e){}
      }
    });
    return send.apply(this, arguments);
  };

  function patchLive(){
    try{
      const ue = window.ue || window.__ue || document.querySelector('canvas')?.__renderer;
      if(ue && ue.current && ue.current.renderer && ue.current.renderer._isMod===false){
        ue.current.renderer.setIsMod(true);
        ue.current.renderer._isMod=true;
      }
      document.querySelectorAll('canvas').forEach(c=>{
        if(c._renderer && c._renderer._isMod===false) c._renderer.setIsMod(true);
      });
    }catch(e){}
    const stores = [window.store, window.__store, window._store, window.__REDUX_STORE__].filter(Boolean);
    stores.forEach(s=>{
      try{
        const st=s.getState?s.getState():null;
        if(st){
          if(st.user && st.user.role===0) st.user.role=200;
          if(st.currentUser && st.currentUser.role===0) st.currentUser.role=200;
          if(st.auth && st.auth.user && st.auth.user.role===0) st.auth.user.role=200;
        }
      }catch(e){}
    });
    ['__INITIAL_DATA__','__INITIAL_STATE__','initialState','__PRELOADED_STATE__'].forEach(k=>{
      try{
        const v=window[k];
        if(v && typeof v==='object'){
          const patch=(o)=>{ if(!o||typeof o!=='object')return; if('role' in o) o.role=200; for(const kk in o) patch(o[kk]); };
          patch(v);
        }
      }catch(e){}
    });
    const btn=document.getElementById('btn-modtools');
    if(btn){
      btn.style.display=''; btn.style.visibility='visible'; btn.style.opacity='1';
      btn.style.pointerEvents='auto';
    }
  }
  setInterval(patchLive, 600);
  const css=document.createElement('style');
  css.textContent=`#btn-modtools{display:flex !important; visibility:visible !important;}
  #panel-modtools{display:block !important;}`;
  (document.head||document.documentElement).appendChild(css);
  console.log('[native-mod] role 200 patch aktif - pixmap.fun kendi butonu açılacak. server 403 verirse yetki DB\'de değil, ama buton native olarak görünür.');
})();
