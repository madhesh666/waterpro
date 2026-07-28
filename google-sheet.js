// ======================================
// Google Apps Script URL
// ======================================

const API_URL = "https://script.google.com/macros/s/AKfycbw5752QiYeF3a-TLokPXi_5beSpjjsrR7oADiilHlbLF8i0ks_LoBIfSeG_h6_NVHzM0g/exec";


// ======================================
// Current Date & Time
// ======================================

function updateDateTime() {

    const now = new Date();

    document.getElementById("currentDate").textContent =
        now.toLocaleDateString();

    document.getElementById("currentTime").textContent =
        now.toLocaleTimeString();

}

updateDateTime();

setInterval(updateDateTime,1000);


// ======================================
// Live Data
// ======================================

async function loadLiveData() {

    try {

        const response =
            await fetch(API_URL + "?action=latest");

        const data =
            await response.json();
            // Live Values
document.getElementById("livePh").textContent = data.phValue;
document.getElementById("liveTds").textContent = data.tdsValue;
document.getElementById("liveEc").textContent = data.ecValue;
document.getElementById("liveTurbidity").textContent = data.turbidityValue;

// Last Updated
console.log("Response:", data);
console.log("Timestamp:", data.timestamp);

const lastUpdate = new Date(data.timestamp);

console.log("Parsed Date:", lastUpdate);
document.getElementById("lastUpdated").textContent =
lastUpdate.toLocaleString();

// System Status
const now = new Date();
const diffMinutes = (now - lastUpdate) / (1000 * 60);

if (diffMinutes <= 60) {
    document.getElementById("systemStatus").innerHTML =
    "🟢 ON";
} else {
    document.getElementById("systemStatus").innerHTML =
    "🔴 OFF";
}

        document.getElementById("livePh").textContent =
            data.phValue ?? "--";

        document.getElementById("liveTds").textContent =
            data.tdsValue ?? "--";

        document.getElementById("liveTurbidity").textContent =
            data.turbidityValue ?? "--";

        document.getElementById("liveEc").textContent =
            data.ecValue ?? "--";

        document.getElementById("systemStatus").innerHTML =
            "🟢 ON";

    }

    catch(error){

        console.log(error);

        document.getElementById("systemStatus").innerHTML =
            "🔴 OFF";

    }

}


// ======================================
// Historical Report
// ======================================

async function loadReport(){

    const fromDate =
        document.getElementById("fromDate").value;

    const toDate =
        document.getElementById("toDate").value;

    const fromTime =
        document.getElementById("fromTime").value;

    const toTime =
        document.getElementById("toTime").value;

    if(
        fromDate=="" ||
        toDate=="" ||
        fromTime=="" ||
        toTime==""
    ){

        alert("Select Date and Time");

        return;

    }

    try{

        const url =
        API_URL +
        "?action=report" +
        "&fromDate=" + encodeURIComponent(fromDate) +
        "&toDate=" + encodeURIComponent(toDate) +
        "&fromTime=" + encodeURIComponent(fromTime) +
        "&toTime=" + encodeURIComponent(toTime);

        const response =
            await fetch(url);

        const data =
            await response.json();

        document.getElementById("reportPh").textContent =
            data.phValue ?? "--";

        document.getElementById("reportTds").textContent =
            data.tdsValue ?? "--";

        document.getElementById("reportTurbidity").textContent =
            data.turbidityValue ?? "--";

        document.getElementById("reportEc").textContent =
            data.ecValue ?? "--";

    }

    catch(error){

        console.log(error);

        alert("Unable to load report");

    }

}


// ======================================
// Button Click
// ======================================

document
.getElementById("getReportBtn")
.addEventListener("click",loadReport);


// ======================================
// Auto Refresh
// ======================================

loadLiveData();

setInterval(loadLiveData,10000);
