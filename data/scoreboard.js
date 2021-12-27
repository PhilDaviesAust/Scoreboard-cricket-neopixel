let interval = setInterval(displayTime, 1000);
let timeupdate = setInterval(updateTime, 60 * 1000);

function updateTime(){
    let d = new Date();
    let seconds = d.getHours()*3600 + d.getMinutes()*60 + d.getSeconds();
    let xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function(){
        if(this.readyState == 4 && this.status == 200){
            document.getElementById("responseframe").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/time?seconds=" + seconds, true);
    xhttp.send();
}

function displayTime(){				//current time of day in seconds
    const d = new Date();
    const seconds = d.getHours()*3600 + d.getMinutes()*60 + d.getSeconds(); 
    document.getElementById("seconds").value = seconds;
}

function blurHandler(tbox){		    //validate input boxes
    let val = Math.abs(parseInt(tbox.value));
    isNaN(val) ? tbox.value = 0: tbox.value = val;
}

function clickHandler(but){		    //up down buttons
    let upDown = parseInt(but.getAttribute("data-dir"));
    let scb = but.parentNode.querySelectorAll(".inputScoreBox");
    scb[0].value = parseInt(scb[0].value) + upDown;
    if(scb[0].value < 0) scb[0].value = "0";
}
