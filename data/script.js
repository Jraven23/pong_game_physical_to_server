/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

let scene, camera, rendered, cube1, cube2, cubeBall, borderTop, borderBottom;

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function init3D(){
  scene = new THREE.Scene();
  scene.background = new THREE.Color(0xffffff);

  camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3DPong")) / parentHeight(document.getElementById("3DPong")), 0.1, 1000);

  renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setSize(parentWidth(document.getElementById("3DPong")), parentHeight(document.getElementById("3DPong")));

  document.getElementById('3DPong').appendChild(renderer.domElement);

  // Create a geometry
  const geometryRaquets = new THREE.BoxGeometry(1, 10, 1);
  const geometryBall = new THREE.BoxGeometry(1, 1, 1);
  const geometryBorder = new THREE.BoxGeometry(400, 0.5, 0.5);

  // Materials of each face
  var cubeMaterials = [
    new THREE.MeshBasicMaterial({color:0x03045e}),
    new THREE.MeshBasicMaterial({color:0x023e8a}),
    new THREE.MeshBasicMaterial({color:0x0077b6}),
    new THREE.MeshBasicMaterial({color:0x03045e}),
    new THREE.MeshBasicMaterial({color:0x023e8a}),
    new THREE.MeshBasicMaterial({color:0x0077b6}),
  ];

  const material = new THREE.MeshFaceMaterial(cubeMaterials);

  cube1 = new THREE.Mesh(geometryRaquets, material);
  cube2 = new THREE.Mesh(geometryRaquets, material);
  cubeBall = new THREE.Mesh(geometryBall, material);
  borderTop = new THREE.Mesh(geometryBorder, material);
  borderBottom = new THREE.Mesh(geometryBorder, material);
  scene.add(cube1,cube2,cubeBall,borderTop,borderBottom);
  //scene.add(cube2);
  //scene.add(cubeBall);
  camera.position.z = 150;
  cube1.position.x = -200;
  cube2.position.x = 200;
  borderTop.position.y = 100;
  borderBottom.position.y = -100;
  renderer.render(scene, camera);
}

// Resize the 3D object when the browser window changes size
function onWindowResize(){
  camera.aspect = parentWidth(document.getElementById("3DPong")) / parentHeight(document.getElementById("3DPong"));
  //camera.aspect = window.innerWidth /  window.innerHeight;
  camera.updateProjectionMatrix();
  //renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setSize(parentWidth(document.getElementById("3DPong")), parentHeight(document.getElementById("3DPong")));

}

window.addEventListener('resize', onWindowResize, false);

// Create the 3D representation
init3D();

// Create events for the game readings
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('pong_data', function(e) {
    //console.log("gyro_readings", e.data);
    var obj = JSON.parse(e.data);

    document.getElementById("player1Points").innerHTML = obj.player1Points;
    document.getElementById("player2Points").innerHTML = obj.player2Points;
    document.getElementById("ballSpeed").innerHTML = obj.ballSpeed;
    //document.getElementById("raquetLength").innerHTML = obj.raquetLength;

    // Change positions receiving the readinds
    cube1.position.y = obj.player1Pos-400;
    cube2.position.y = obj.player2Pos-400;
    cubeBall.position.x = obj.ballPosX-400;
    cubeBall.position.y = obj.ballPosY-400;
    
    renderer.render(scene, camera);
  }, false);

}

function restartGame(){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/restart", true);
  console.log("restart");
  xhr.send();
}

function changeSpeed(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/"+element.id, true);
  console.log(element.id);
  xhr.send();
}