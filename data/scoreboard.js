let interval = setInterval(displayTime, 1000);

function displayTime(){				//current time of day in seconds
    const d = new Date();
    let seconds = d.getHours()*3600 + d.getMinutes()*60 + d.getSeconds(); 
    document.getElementById("seconds").value = seconds;
}

function blurHandler(tbox){		    //validate input boxes
    let val = Math.abs(parseInt(tbox.value));
    isNaN(val) ? tbox.value = 0: tbox.value = val;
    let mod = (tbox.id == "score" || tbox.id == "target") ? 1000: 100;
    tbox.value = tbox.value%mod;
}

function clickHandler(btn){		    //up down buttons
    let upDown = parseInt(btn.getAttribute("data-dir"));
    let scbox = btn.parentNode.querySelector(".inputScoreBox");
    scbox.value = parseInt(scbox.value) + upDown;
    if(scbox.value < 0) scbox.value = "0";
    let mod = (scbox.id == "score" || scbox.id == "target") ? 1000: 100;
    scbox.value = scbox.value%mod;
}
