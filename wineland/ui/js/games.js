/*
 * Copyright 2020 varmd
*/
//import {default as low} from './lodash.min.js';



function Uint8ToString(u8a){
    var CHUNK_SZ = 0x8000;
    var c = [];
    for (var i=0; i < u8a.length; i+=CHUNK_SZ) {
      c.push(String.fromCharCode.apply(null, u8a.subarray(i, i+CHUNK_SZ)));
    }
    return c.join("");
}

//Strings

const ConfFile = (options) => `
${options.gdi ? '' : 'WINE_VK_VULKAN_ONLY=1' }
${options.fullscreen ? 'WINE_VK_ALWAYS_FULLSCREEN=1' : '' }
${options.fullscreen_grab ? 'WINE_VK_FULLSCREEN_GRAB_CURSOR=1' : '' }
${options.custom_cursors ? 'WINE_VK_USE_CUSTOM_CURSORS=1' : '' }
${options.winefsync ? 'WINEFSYNC=1' : 'WINEFSYNC=0' }
${options.wineesync ? 'WINEESYNC=1' : '' }
${options.fsr ? 'WINE_VK_USE_FSR=1' : '' }

MANGOHUD=${options.mangohud  ? 1 : ''}
MANGOHUD_CONFIG="${options.mangohud_config  ? options.mangohud_config : ''}"
GAME_PATH="${options.game_path}"
GAME_EXE="${options.game_exe}"
GAME_OPTIONS="${options.exe_options}"

${options.width ? "WINE_VK_WAYLAND_WIDTH=" + options.width  : '' }
${options.height ? "WINE_VK_WAYLAND_HEIGHT=" + options.height  : '' }
${options.cursor_size ? "XCURSOR_SIZE=" + options.cursor_size  : '32' }
`;



const ListPage = (objs) => `
  <!--<h3>Games</h3>-->
    <div class="container-fluid">
    
      <div class="row">
       ${ objs.map(obj => 
      `
      <div class="col-sm-4">
      <div class="card">
        <h4 class="card-title">
          ${ obj.title }
        </h4>
        <p>
          ${ obj.content }
        </p>
        <div class="text-right">
          <button class="edit-link btn" style="cursor: pointer; font-weight: bold;" data-name="${ obj.name }" href="#">Edit</button>
          <button type="button" class="launch-link btn" style="cursor: pointer; font-weight: bold;${ obj.options.exepath ? '' : 'display: none;' }" data-name="${ obj.name }" href="#">Launch</button>
        </div>
      </div>
      
      </div>
      
      `
      ).join('') }
  </div>

`;

const DocForm = (obj) => `
  <form class="col-8">
        <h4>Edit ${ obj.name }</h4>

        
        <input type="hidden" name="name" value="${ obj.name }" />

        

              <div class="form-group">
                <label for="title" class="required">Title</label>
                <input type="text" name="title" value="${ obj.title }" required class="form-control" placeholder="Title">
              </div>
              
              <div class="form-group">
                <label for="exepath" class="required">EXE path (example Bin/file.exe)</label>
                <input required type="text" name="exepath" value="${ obj.options.exepath }" class="form-control" placeholder="EXE path">
              </div>
              
              
              <div class="form-group">
                <label for="exe_options" >EXE options (example -option)</label>
                <input type="text" name="exe_options" value="${ obj.options.exe_options }"  class="form-control" placeholder="EXE options">
              </div>

              <div class="form-group">
              
                <div class="custom-checkbox">
                  <input type="checkbox" name="fullscreen" id="checkbox-1" value="1" ${ obj.options.fullscreen ? 'checked="checked"' : '' }>
                  <label for="checkbox-1">Wayland Fullscreen</label>
                </div>
              </div>
                  
              
              <div class="form-group">  
                <div class="custom-checkbox">
                  <input type="checkbox" name="fullscreen_grab" id="checkbox-2" value="1" ${ obj.options.fullscreen_grab ? 'checked="checked"' : '' }>
                  <label for="checkbox-2">Fullscreen Cursor Grab</label>
                </div>
              </div>  
                  
                <div class="form-group">  
                
                <div class="custom-checkbox">
                  <input type="checkbox" name="mangohud" id="checkbox-3" value="1" ${ obj.options.mangohud ? 'checked="checked"' : '' }>
                  <label for="checkbox-3">Enable Mangohud</label>
                </div>
                  
              </div>

                <div class="form-group">
                  <label for="mangohud_config" >Mangohud config</label>
                  <input type="text" name="mangohud_config" value="${ obj.options.mangohud_config }"  class="form-control" placeholder="Mangohud config">
                </div>
                    
              </div>
              
              <div class="form-group">                  
                <div class="custom-checkbox">
                  <input type="checkbox" name="gdi" id="checkbox-4" value="1" ${ obj.options.gdi ? 'checked="checked"' : '' }>
                  <label for="checkbox-4">Enable GDI (error popups, launchers) - may break some games</label>
                </div>
              </div>
                  
              <div class="form-group">                  
                <div class="custom-radio">
                  <input type="radio" name="winesync" id="winefsync" value="winefsync" ${ obj.options.winefsync && !obj.options.wineesync && !obj.options.winenosync ? 'checked="checked"' : '' } >
                  <label for="winefsync">FSYNC</label>
                </div>

                <div class="custom-radio">
                  <input type="radio" name="winesync" id="wineesync" ${ !obj.options.winefsync && obj.options.wineesync && !obj.options.winenosync ? 'checked="checked"' : '' } value="wineesync">
                  <label for="wineesync">ESYNC</label>
                </div>
                  
                <div class="custom-radio">
                  <input type="radio" name="winesync" id="winenosync" ${ !obj.options.winefsync && !obj.options.wineesync && obj.options.winenosync ? 'checked="checked"' : '' } value="winenosync">
                  <label for="winenosync">Disable FSYNC/ESYNC</label>
                </div>
              </div>

              <div class="form-group">     
                <textarea required class="form-control" name="content"  placeholder="Description" rows="3" >${ obj.content }</textarea>
              </div>
                
              <div class="form-group">     
                <label for="width" class="required">Width</label>
                <input type="number" name="width" value="${ obj.options.width }" required class="form-control" placeholder="Width">  
              </div>
                
              <div class="form-group">     
                <label for="height" class="required">Height</label>
                <input type="number" name="height" value="${ obj.options.height }" required class="form-control" placeholder="Height">  
              </div>
                
              <div class="form-group">     
                <label for="cursor_size" class="required">Cursor Size</label>
                <input type="number" name="cursor_size" value="${ obj.options.cursor_size }" required class="form-control" placeholder="Cursor Size">  
              </div>
                
              <div class="form-group">  
                <div class="custom-checkbox">
                  <input type="checkbox" name="custom_cursors" id="checkbox-custom-cursors" value="1" ${ obj.options.custom_cursors ? 'checked="checked"' : '' }>
                  <label for="checkbox-custom-cursors">Enable custom game cursors (disables cursor size)</label>
                </div>
              </div>  
                  
              <div class="form-group">  
                <div class="custom-checkbox">
                  <input type="checkbox" name="fsr" id="checkbox-fsr" value="1" ${ obj.options.fsr ? 'checked="checked"' : '' }>
                  <label for="checkbox-fsr">Enable FSR (enabled Wayland fullscreen)</label>
                </div>
              </div>  
          
          
          
  
          
          
          <div class="form-group">   
            <div class="col-2 column">
              <button class=" btn btn-primary btn-lg">Submit</button>
            </div>
          </div>
              
        
      </form>
`;



//basic funcs
var $id = function(id) {
  id = id.replace(/^#/,'');
  var  el = document.getElementById(id);
  if(!el) {
    el = {};el.style = {};el.innerHTML='';el.size = 0;
    el.addEventListener = () => {};
  }
  return el;
}

var $prepend = function(el, html) {
  el.innerHTML = html + el.innerHTML;
}

var $remove = function(el) {
  if(el.size === 0) {
    return;
  }
  
  el.parentNode.removeChild(el);  
}

var $sel = function(sel) {
  var p = document.querySelectorAll(sel);
  if(!p) { 
    p = [{addEventListener: ()=>{}}];    
  }
  var n = [];
  for (var i = 0; i < p.length; ++i) {
      n[i] = p[i];
  }
  
  return n;
}

var $event = function(el, event, f) {
  
  if(el) {
    el.addEventListener(event, f);
  }
}


var $del_class = function(els, rm_cl) {
  if(!els) return;
  
  for(var i in els) {
    var el = els[i];
    var cl = el.getAttribute("class");
    var re = new RegExp(rm_cl, "g");
    cl = cl.replace(re, "");
    el.setAttribute("class", cl);    
  }
}


var $add_class = function(el, add) {
  if(!el || !el.getAttribute) return;
  var cl = el.getAttribute("class");
  el.setAttribute("class", cl + " " + add);
  return el;
}


function basename(path) {
   return path.split('/').reverse()[0];
}

//


//lodash adapter
//todo remove lodash
var global_json = "";
var global_iid = 0;
var global_existing_games = [];

class localStorageServer {
  constructor() {
    this.source = ""
    this.defaultValue = ""
  }

  read() {
    
    this.defaultValue = global_json;
    return global_json;
  }

  write(data) {
    // Should return nothing or a Promise
    
    if(!data) return;
    
    if(global_iid) {
      clearTimeout(global_iid);
    }
    
    var str = JSON.stringify(data);
    if(str == "") {
      return;
    }
    
    global_iid = setTimeout(() => {
      console.log("writing...");
      console.log(data);
      
      clearTimeout(global_iid);
      global_iid = null;
      fetch('/save-json', {
        method: 'post',
        headers: {
          'Accept': 'application/json, text/plain, */*',
          'Content-Type': 'application/json'
        },
        body: str
      }).then(res => console.log(res));
    }
    ,500);
    
    
  }
}



//import * as data from './data.json'


fetch("/list-games")
     .then(response => {
       if(response.ok) return response.text()
      } 
     )
     .then(text => {
       
       if(text) {
         
         if(text) {
           text = text.replace("old\n","").replace("archive\n","").replace("trash\n","");
           global_existing_games = text.split("\n");    
           console.log(global_existing_games);
           global_existing_games.pop();
           load_json();

         }
         //load_db(json); 
        } else {
          //load_db({}); 
        }
});

function load_json() {
  fetch("/load-file?data.json")
       .then(response => {
         if(response.ok) 
           return response.json()
         else
          load_db({});  
        } 
       )
       .then(json => {
         
         if(json) {
           console.log("loading");
           load_db(json); 
          } else {
            load_db({}); 
          }
  });
}


var db, db_adapter;

function load_db(json) {
  global_json = json;
  db_adapter = new localStorageServer("");
  db = low(db_adapter);
  
  load_ui();
}


function load_ui(type, data) {
  
  
  
  
  if(!db.get('games').value() || db.get('games').value().length < 1) {
    db.defaults({ "games": [] }).write();
  }  
    
  console.log("inserting data");
  console.log(global_existing_games);
  console.log("inserting data 22");
  var games = [];
  for(var i in global_existing_games) {
    console.log(  global_existing_games[i] );
    
    obj = db.get('games').find({ name: global_existing_games[i] }).value();
    console.log(obj);
    if(!obj) {
      games.push({
        time: Date.now() + 11 + i,
        name: global_existing_games[i],
        title: global_existing_games[i],
        content: global_existing_games[i],
        options: {},
      });
    } else {
      games.push(obj);
    }
  
  }
  
  db.get('games').remove().value();
  db.get('games').assign(games).value();
  
  db.write();
  
    

  
  load_docs();
  
    
}




//Load objs
function load_docs(){
  
  var objs = db.get('games').value();
  
  $id("content-area").innerHTML = ListPage(objs);
  
  
  var tbl_name = "games";
  
  var f = (o) => {
    
    var obj = {time: "", content: "", "title": "" };
    
    if(o) {
      o.content = o.content.replace(/<br\/>/g, "\r");
      obj = o;  
    }
    
    
    
    var defaults = {
        exepath: "",
        exe_options: "",
        fullscreen: 0,
        fullscreen_grab: 0,
        mangohud: 0,
        mangohud_config: "",
        gdi: 0,
        dxvk_hud: 0,
        winefsync: 1,
        wineesync: 0,
        winenosync: 0,
        width: 1920,
        height: 1080,
        cursor_size: 32,
        custom_cursors: 0,
        fsr: 0,
    };
    
    
    if(!obj.options){
      obj.options = defaults;
    } else {
      for(var i in defaults) {
        if(!obj.options.hasOwnProperty(i)) {
          obj.options[i] = defaults[i];
        }
      }
    }
    
    console.log(obj);
    
    
    $id("content-area").innerHTML = DocForm(obj);
    
    
    $sel("#content-area form").forEach( (el) => el.addEventListener("submit", (e) => {
      e.preventDefault();
      
      if(!el.name.value) {
        return;  
      }
      
        el.content.value = el.content.value.replace(/[\r\n]/g, "<br/>");
        el.content.value = el.content.value.replace(/<script/g, "");
        el.content.value = el.content.value.replace(/<style/g, "");
      
        let options = {
          exepath: el.exepath.value,  
          exe_options: el.exe_options.value,  
          fullscreen: el.fullscreen.checked,  
          fullscreen_grab: el.fullscreen_grab.checked,  
          mangohud: el.mangohud.checked,          
          mangohud_config: el.mangohud_config.value,          
          gdi: el.gdi.checked,          
          winefsync: el.winesync.value == "winefsync" ? '1' : 0,
          wineesync: el.winesync.value == "wineesync" ? '1' : 0,
          winenosync: el.winesync.value == "winenosync" ? '1' : 0,
          width: el.width.value,
          height: el.height.value,
          cursor_size: el.cursor_size.value,
          custom_cursors: el.custom_cursors.checked,
          fsr: el.fsr.checked,          
        }
      
        console.log("Updating data");
        db.get(tbl_name)
        .find({ name: el.name.value })
        .assign({  
            content: el.content.value,
            title: el.title.value,
            time: Date.now(),
            options: options,
          }
        ).value();
            
        
        options.game_exe = basename(options.exepath);
        options.game_path = options.exepath.replace(options.game_exe,"");
        let game_paths = options.game_path.split("/");
        game_paths = options.game_path.split("/");
        options.game_path = game_paths.shift();
        if(game_paths.length && game_paths[0] != "") {
          options.game_exe = game_paths.join("/") + options.game_exe;
        }
        
        let str = ConfFile(options);
          
        db.write();
        
        
        //write settings
          
        fetch('/save-conf?' + el.name.value, {
          method: 'post',
          headers: {
            'Accept': 'application/json, text/plain, */*',
            'Content-Type': 'application/json'
          },
          body: btoa(str)
        }).then(res => console.log(res));
        
        //end write settings
        
        
        load_docs();
        
        
        
        
      }
     )
    );
      
  }; //end form function
  
  
  
  var btn_edit = $sel(".edit-link");
  var btn_launch = $sel(".launch-link");
  
  btn_edit.forEach( (el) => el.addEventListener("click",
    () => {
      var name = el.getAttribute("data-name"); 
      var obj = db.get(tbl_name)
        .find({ "name": name }).value();    

       
      if(obj && obj.name) {
        f(obj);  
      }
    }
  ));
    
    
  btn_launch.forEach( (el) => el.addEventListener("click",
    () => {
      
      el.setAttribute("disabled", "disabled");
      
      var name = el.getAttribute("data-name"); 
      fetch("/launch-game?" + name);
      
      //re-enable button
      setTimeout(() => {
        el.removeAttribute("disabled");
      }
      , 7000);
      
    }
  ));
  
  
}





