
var mneu_btn = document.getElementsByClassName('ctf01d-global-page-switcher')[0];
var tabs_content = document.getElementsByClassName('ctf01d-page-content');

mneu_btn.onclick = function() {
    // mneu_btn
    for (var i = 0; i < tabs_content.length; i++) {
        tabs_content[i].style.display = '';
    }

    var nextcontentid = this.getAttribute('nextcontentid');
    document.getElementById(nextcontentid).style.display = 'block';

    if (nextcontentid == 'game_details') {
        this.setAttribute('nextcontentid', 'game_scoreboard');
    } else {
        this.setAttribute('nextcontentid', 'game_details');
    }

    console.log(nextcontentid)

}

const escapeHtml = (unsafe) => {
    return unsafe.replaceAll('&', '&amp;').replaceAll('<', '&lt;').replaceAll('>', '&gt;').replaceAll('"', '&quot;').replaceAll("'", '&#039;');
}

function copyToBuffer(elid) {
    var el = document.getElementById(elid);
    el.focus();
    el.select();
    try {
        var successful = document.execCommand('copy');
        var msg = successful ? 'successful' : 'unsuccessful';
        console.log('Copying text command was ' + msg);
    } catch (err) {
        console.log('Oops, unable to copy');
    }
}

var curl_example = document.getElementById("curl_request_send_flag").innerHTML;
curl_example = curl_example.replace("{JURY_HOST_PORT}", window.location);
document.getElementById("curl_request_send_flag").innerHTML = curl_example;

var py_example = document.getElementById("python_request_send_flag").innerHTML;
py_example = py_example.replace("{JURY_HOST_PORT}", window.location);
document.getElementById("python_request_send_flag").innerHTML = py_example;



function get_subnet(ip) {
    var subnet = ip.split(".");
    subnet.pop();
    return subnet.join(".");
}

function updateTeamRequiredFields() {
    getAjax('/api/v1/myip', function(err, resp){
        if (err) {
            console.error("err = ", err, "resp =", resp);
            return;
        }
        window.myip = resp["myip"];
        console.log("MyIP: " + window.myip);
        window.found_teamid = undefined;
        var found_teams = []
        // search by exact match ip
        for (var i = 0; i < window.teams.length; i++) {
            if (window.teams[i].ip_address == window.myip) {
                found_teams.push(window.teams[i].id)
            }
        }
        if (found_teams.length == 0) {
            var mysubnet = get_subnet(window.myip)
            console.log("mysubnet " + mysubnet)
            // search by subnet match
            for (var i = 0; i < window.teams.length; i++) {
                if (get_subnet(window.teams[i].ip_address) == mysubnet) {
                    found_teams.push(window.teams[i].id)
                }
            }
            if (found_teams.length == 1) {
                window.found_teamid = found_teams[0];
                console.log("Detected teamid by subnet: " + window.found_teamid)
            } else if (found_teams.length > 1) {
                console.warn("Could not detected teamid by subnetwork found several teams: " + found_teams.join(", "))
            }
        } else if (found_teams.length == 1) {
            window.found_teamid = found_teams[0];
            console.log("Detected teamid by ip: " + window.found_teamid)
        }

        if (window.found_teamid) {
            document.getElementById('team_list').value = window.found_teamid;

            var curl_example = document.getElementById("curl_request_send_flag").innerHTML;
            curl_example = curl_example.replace("{YOUR_TEAM_ID}", window.found_teamid);
            document.getElementById("curl_request_send_flag").innerHTML = curl_example;

            var py_example = document.getElementById("python_request_send_flag").innerHTML;
            py_example = py_example.replace("{YOUR_TEAM_ID}", window.found_teamid);
            document.getElementById("python_request_send_flag").innerHTML = py_example;

            document.getElementById(window.found_teamid).classList.add('current-team');
        }
    })
}

// post request to server Async
function getAjax (url, callback) {
    callback = callback || function(){};
    var tmpXMLhttp = null;
	if (window.XMLHttpRequest) {
		// code for IE7+, Firefox, Chrome, Opera, Safari
		tmpXMLhttp = tmpXMLhttp || new window.XMLHttpRequest();
	};
	tmpXMLhttp.onreadystatechange=function() {
		if (tmpXMLhttp.readyState==4) {
			if (tmpXMLhttp.responseText == '') {
                // obj = { 'result' : 'fail' };
                callback('fail', null);
            } else {
				try {
					var obj = JSON.parse(tmpXMLhttp.responseText);
                    callback(null, obj);
                    // delete obj;
				} catch(e) {
					console.error(e.name + ':' + e.message + '\n stack:' + e.stack + '\n' + tmpXMLhttp.responseText);
				}
				// delete tmpXMLhttp;
			}
		}
	}
	tmpXMLhttp.open('GET', url, true);
	tmpXMLhttp.send();
};
var scoreboard_content = document.getElementById('scoreboard_content');
var loader_content = document.getElementById('loader_content');

function _animateElement(el, enable) {
    if (el == null) {
        console.error("_animateElement el is null");
        return;
    }
    el.style.animation = enable ? "blinking 0.8s reverse infinite" : '';
}

function _animateElementOneTime(elid) {
    var el = document.getElementById(elid)
    if (el == null) {
        console.error("_animateElementOneTime el is null by id ", elid);
        return;
    }
    el.style.animation = "fastblinking 0.8s reverse infinite";
    var timer2 = setTimeout(function(elid) {
        var el1 = document.getElementById(elid);
        if (!el1) {
            console.err("el1 = ", el1, "elid = ", elid);
        }
        document.getElementById(elid).style.animation = '';
        clearTimeout(timer2);
    },800, elid);
}

function _animateElementServiceCell(elid) {
    var el = document.getElementById(elid)
    if (el == null) {
        console.error("_animateElementServiceCell el is null by id ", elid);
        return;
    }
    var scale_val = 1.0;
    var scale_max_val = 2.0;
    var scale_diff = 0.4;
    // el.style.animation = "fastblinking 0.8s reverse infinite";
    var inter2 = setInterval(function(_el) {
        if (scale_diff > 0) {
            if (scale_val < scale_max_val) {
                scale_val += scale_diff;
                _el.style.transform = 'scale(1, ' + scale_val + ')';
            } else {
                scale_diff = -(scale_diff) / 2.0;
            }
        } else {
            if (scale_val > 1.0) {
                scale_val += scale_diff;
                _el.style.transform = 'scale(1, ' + scale_val + ')';
            } else {
                _el.style.transform = '';
                clearInterval(inter2);
            }
        }
    }, 40, el);
}

function silentUpdate(elid, newValue) {
    var el = document.getElementById(elid)
    if (!el) {
        console.error("Not found element with id " + elid);
        return;
    }
    if (el.innerHTML != newValue) {
        el.innerHTML = newValue;
        _animateElementOneTime(elid);
        // TODO make simple anim
    }
}

function silentUpdateWithoutAnimation(elid, newValue) {
    var el = document.getElementById(elid)
    if (!el) {
        console.error("Not found element with id " + elid);
        return;
    }
    if (el.innerHTML != newValue) {
        el.innerHTML = newValue;
    }
}

function silentUpdateWidthWithoutAnimation(elid, newValue) {
    var el = document.getElementById(elid)
    if (!el) {
        console.error("Not found element with id " + elid);
        return;
    }
    if (el.style.width != newValue) {
        el.style.width = newValue;
    }
}

var g_is_showed_authomation = false;

function showActionAutomatization() {
    if (g_is_showed_authomation) {
        return;
    }
    g_is_showed_authomation = true;
    var w = window.innerWidth;
    var h = window.innerHeight;
    var size_min_persent = 0.25;
    var size_max_persent = 0.55;
    var size_percent = Math.random() * (size_max_persent - size_min_persent) + size_min_persent;
    var size_px = size_percent * w;
    var top_px = Math.random() * (h - size_px);
    var left_px = Math.random() * (w - size_px);

    var new_id = "mass_action_" + Math.random()*10000;
    document.getElementById('game_scoreboard').innerHTML +=
        '<div id="' + new_id + '" class="mass-action mass-action-automatization" '
        + ' style="top: ' + top_px + 'px; left: ' + left_px + 'px; width: ' + size_px + 'px; height: ' + size_px + 'px;"'
        + '></div>';

    var timer_automatization_2 = setTimeout(function(new_id) {
        var node = document.getElementById(new_id);
        node.parentNode.removeChild(node);
        clearTimeout(timer_automatization_2);
        g_is_showed_authomation = false;
    }, 2400, new_id);
}

function showActionFirstblood(teamId) {
    var el = document.getElementById(teamId);
    if (el) {
        el.style.animation = "team-first-blood 0.8s cubic-bezier(0, 0.6, 0.7, 1.0) infinite";
        var timer_first_blood_2 = setTimeout(function(teamId1) {
            document.getElementById(teamId1).style.animation = "";
            clearTimeout(timer_first_blood_2);
        }, 2500, teamId);
    }

    var new_first_blood_id = "mass_action_" + Math.random()*10000;
    var gm = document.getElementById('game_scoreboard')
    gm.innerHTML +=
        '<div id="' + new_first_blood_id + '" class="mass-action mass-action-firstblood" '
        + ' style="top: 0px; left: 0px; width: 10%; height: 1%;"'
        + '></div>';

    var timer_first_blood_1 = setTimeout(function(new_first_blood_id) {
        var node = document.getElementById(new_first_blood_id);
        gm.removeChild(node);
        clearTimeout(timer_first_blood_1);
    }, 5000, new_first_blood_id);
}

function updateUIValue(t, teamID, paramName){
    var newValue = '';
    if (paramName == 'points') {
        newValue = t[paramName].toFixed(1);
    } else {
        newValue = '' + t[paramName];
    }
    var elem_id = paramName + '-' + teamID;
    var el = document.getElementById(elem_id);
    if (el) {
        var prevVal = el.innerHTML;
        if (prevVal != newValue) {
            if (paramName == "tries") {
                if (prevVal != "") {
                    var diff = parseInt(newValue, 10) - parseInt(prevVal, 10);
                    // console.log("diff", diff)
                    if (diff >= 5) {
                        showActionAutomatization();
                    }
                    _animateElement(document.getElementById('tries-icon-' + teamID), true);
                    if (diff != 0) {
                        newValue += " +" + diff;
                    }
                }
            }
            if (paramName == "place") {
                if (newValue != '3' && newValue != '2' && newValue != '1') {
                    document.getElementById(elem_id).innerHTML = newValue;
                }
            } else {
                document.getElementById(elem_id).innerHTML = newValue;
            }
        } else {
            if (paramName == "tries") {
                _animateElement(document.getElementById('tries-icon-' + teamID), false);
            }
        }
        if (paramName == "place" && prevVal != newValue) {
            if (newValue == "1") {
                el.classList.remove('place-2st');
                el.classList.remove('place-3st');
                if (!el.classList.contains('place-1st')) {
                    el.classList.add('place-1st');
                }
            } else if (newValue == "2") {
                el.classList.remove('place-1st');
                el.classList.remove('place-3st');
                if (!el.classList.contains('place-2st')) {
                    el.classList.add('place-2st');
                }
            } else if (newValue == "3") {
                el.classList.remove('place-1st');
                el.classList.remove('place-2st');
                if (!el.classList.contains('place-3st')) {
                    el.classList.add('place-3st');
                }
            } else {
                el.classList.remove('place-1st');
                el.classList.remove('place-2st');
                el.classList.remove('place-3st');
            }
        }
    } else {
        console.error('Not found element: ' + elem_id);
    }
};

var inSwitch = false;

function switchUITeamRows(teamID1, teamID2){
    console.log('switchUITeamRows: ' + teamID1 + ' <-> ' + teamID2);
    if (inSwitch) {
    /*    setTimeout(function(teamID1, teamID2){
            switchUITeamRows(teamID1, teamID2);
        },50);*/
        return;
    }
    inSwitch = true;
    var el1 = document.getElementById(teamID1);
    var el2 = document.getElementById(teamID2);

    el1.style.transform = 'translateY(100px)';
    // el2.style.transform = 'translateY(-100px)';
    var timeout1 = setTimeout(function(){
        el2.parentNode.insertBefore(el2, el1);
        el1.style.transform = '';
        el2.style.transform = '';
        inSwitch = false;
        clearTimeout(timeout1);
    },400);
}

function helpNumWorld(value, words) {
    if (value == 1) return words[0];
    return words[1];
}

function humanTimeFromSeconds(sec) {
    var one_day_in_seconds = 24*60*60;
    var all_days = Math.round(sec / one_day_in_seconds);
    var years = Math.round(all_days / 356);
    var months = Math.round(all_days / 30) % 12;
    var days = all_days % 30;
    var result = "";
    if (years > 0) {
        result += years + " " + helpNumWorld(years, ["year", "years"])
    }
    if (months > 0) {
        result += " ";
        result += months + " " + helpNumWorld(months, ["month", "months"])
    }
    if (days > 0) {
        result += " ";
        result += days + " " + helpNumWorld(days, ["day", "days"])
    }
    var val = sec % one_day_in_seconds;
    // console.log(val)
    var seconds = val % 60;
    val = val - seconds;
    val = Math.round(val / 60);
    var minutes = val % 60;
    val = val - minutes;
    val = Math.round(val / 60);
    var hours = val % 24;

    if (hours > 0) {
        result += " ";
        result += hours + " " + helpNumWorld(days, ["hour", "hours"])
    }
    if (minutes > 0) {
        result += " ";
        result += minutes + " " + helpNumWorld(minutes, ["minute", "minutes"])
    }

    if (minutes > 0) {
        result += " ";
        result += seconds + " " + helpNumWorld(seconds, ["second", "seconds"])
    }

    if (result.trim() == "") {
        result = "NOW!";
    }
    return result
}

function updateScoreboard() {
    getAjax('/api/v1/scoreboard', function(err, resp){
        if (err) {
            document.getElementById('scoreboard_content').style.display = 'none';
            document.getElementById('loader_content').style.display = 'block';
            console.error("err = ", err, "resp =", resp);
            return;
        }
        // console.log(resp);
        for (var serviceId in resp.s_sta) {
            var s = resp.s_sta[serviceId]
            var firstBloodId = serviceId + '-first-blood';
            var firstBloodTeamName = serviceId + '-first-blood-teamname';
            var firstBloodTime = serviceId + '-first-blood-time';
            var prevValue = document.getElementById(firstBloodId).innerHTML;
            var newValue = s.first_blood;
            var firstBloodTimeFromStartGame = "-";
            if (s.first_blood_ts != 0) {
                firstBloodTimeFromStartGame = humanTimeFromSeconds(s.first_blood_ts - resp.game.t0);
            }

            for (var teamN in document.ctf01d_teams) {
                if (document.ctf01d_teams[teamN].id == s.first_blood) {
                    newValue = escapeHtml(document.ctf01d_teams[teamN].name);
                    break;
                }
            }
            if (prevValue == "-") {
                silentUpdateWithoutAnimation(firstBloodId, newValue);
                silentUpdateWithoutAnimation(firstBloodTeamName, newValue);
                silentUpdateWithoutAnimation(firstBloodTime, firstBloodTimeFromStartGame);
            } else if (prevValue != newValue) {
                silentUpdate(firstBloodId, newValue);
                silentUpdate(firstBloodTeamName, newValue);
                silentUpdate(firstBloodTime, firstBloodTimeFromStartGame);
                showActionFirstblood(s.first_blood);
            }
            silentUpdateWithoutAnimation(serviceId + '-all-flags-att', s.af_att)
            silentUpdateWithoutAnimation(serviceId + '-all-flags-def', s.af_def)
        }

        // game time
        var game_len_time = resp.game.t3 - resp.game.t0;
        var game_passed_time = resp.game.tc - resp.game.t0;
        // console.log("game_len_time", game_len_time);
        if (resp.game.tc < resp.game.t0) {
            silentUpdateWithoutAnimation(
                'game_current_time',
                'game started after: ' + humanTimeFromSeconds(resp.game.t0 - resp.game.tc)
            );
            document.getElementById('game_progress_time').style.display = 'none';
        } else if (resp.game.tc >= resp.game.t1 && resp.game.tc <= resp.game.t2) { // coffee break
            silentUpdateWithoutAnimation(
                'game_current_time',
                'the game will continue after the coffee break in ' + humanTimeFromSeconds(resp.game.t2 - resp.game.tc)
            );
        } else if (resp.game.tc > resp.game.t3) {
            silentUpdateWithoutAnimation('game_current_time', 'game ended');
            document.getElementById('game_progress_time').style.display = 'block';
            document.getElementById('game_progress_time').style.width = '100%';
        } else if (
            resp.game.t1 > resp.game.t0 && resp.game.t1 < resp.game.t3
            && resp.game.t2 > resp.game.t0 && resp.game.t2 < resp.game.t3
        ) { // coffee break enabled
            // console.log("game passed_time", (game_passed_time / game_len_time)*100);
            document.getElementById('game_progress_time').style.display = 'block';
            document.getElementById('game_progress_time').style.width = Math.ceil((game_passed_time / game_len_time)*100) + '%';
            if (resp.game.tc > resp.game.t0 && resp.game.tc < resp.game.t1) { // before coffee break
                silentUpdateWithoutAnimation(
                    'game_current_time',
                    'game time: ' + humanTimeFromSeconds(resp.game.tc - resp.game.t0) + ' and coffee break will start in ' + humanTimeFromSeconds(resp.game.t1 - resp.game.tc)
                );
                document.getElementById('game_progress_time').style.display = 'block';
            } else if (resp.game.tc > resp.game.t2 && resp.game.tc < resp.game.t3) { // after coffee break
                silentUpdateWithoutAnimation(
                    'game_current_time',
                    'game time: ' + humanTimeFromSeconds(resp.game.tc - resp.game.t0) + ' and game will end in ' + humanTimeFromSeconds(resp.game.t3 - resp.game.tc)
                );
            }
        } else if (resp.game.tc > resp.game.t0 && resp.game.tc < resp.game.t3) { // before coffe break
            silentUpdateWithoutAnimation('game_current_time', 'game time: ' + humanTimeFromSeconds(resp.game.tc - resp.game.t0) + ', and game will end in ' + humanTimeFromSeconds(resp.game.t3 - resp.game.tc));
            document.getElementById('game_progress_time').style.display = 'block';
            document.getElementById('game_progress_time').style.width = Math.ceil((game_passed_time / game_len_time)*100) + '%';
        }

        var teamIDs = [];
        for(var teamID in resp.scoreboard){
            var t = resp.scoreboard[teamID];
            teamIDs.push(teamID);
            var teamLogoElemId = "team-logo-" + teamID;
            var lastWriteTimeLogo = document.getElementById(teamLogoElemId).getAttribute('logo_last_updated');
            if (lastWriteTimeLogo == "0") {
                document.getElementById(teamLogoElemId).setAttribute('logo_last_updated', t.logo_last_updated);
            } else if (lastWriteTimeLogo != t.logo_last_updated) {
                console.warn("Need update logo for team ", t);
                document.getElementById(teamLogoElemId).setAttribute('logo_last_updated', t.logo_last_updated);
                var logoUrl = document.getElementById(teamLogoElemId).src;
                if (logoUrl.indexOf("?") !== -1) {
                    logoUrl = logoUrl.split("?")[0];
                }
                document.getElementById(teamLogoElemId).src = logoUrl + "?t=" + t.logo_last_updated;
            }

            var elPointsTrend = document.getElementById(teamID + '-points-trend');
            var prevPoints = parseFloat(document.getElementById(teamID + '-points').innerHTML);
            var newPoints = parseFloat(t.points.toFixed(1));
            if (elPointsTrend.innerHTML == "??") {
                elPointsTrend.classList.add("trend-middle")
                elPointsTrend.classList.remove("trend-up")
                elPointsTrend.classList.remove("trend-down")
                elPointsTrend.innerHTML = "+0";
            } else {
                if (newPoints == prevPoints) {
                    elPointsTrend.classList.add("trend-middle")
                    elPointsTrend.classList.remove("trend-up")
                    elPointsTrend.classList.remove("trend-down")
                    elPointsTrend.innerHTML = "+0";
                } else if (newPoints > prevPoints) {
                    elPointsTrend.classList.remove("trend-middle")
                    elPointsTrend.classList.add("trend-up")
                    elPointsTrend.classList.remove("trend-down")
                    elPointsTrend.innerHTML = "+" + (newPoints - prevPoints).toFixed(1);
                } else {
                    elPointsTrend.classList.remove("trend-middle")
                    elPointsTrend.classList.remove("trend-up")
                    elPointsTrend.classList.add("trend-down")
                    elPointsTrend.innerHTML = "-" + (prevPoints - newPoints).toFixed(1);
                }
            }
            silentUpdate(teamID + '-points', newPoints.toFixed(1));

            updateUIValue(t, teamID, 'place');
            // updateUIValue(t, teamID, 'points');
            updateUIValue(t, teamID, 'tries');
            for(var sService in t.ts_sta){
                var newState = t.ts_sta[sService]['status'];
                var newAttackFlags = t.ts_sta[sService]['att'];
                // var newDefenceFlags = t.ts_sta[sService]['def'];
                var newAttackPoints = t.ts_sta[sService]['pt_att'];
                var newDefencePoints = t.ts_sta[sService]['pt_def'];
                var newSLA = t.ts_sta[sService]['sla'];
                var elId = 'status-' + teamID + '-' + sService;
                var el = document.getElementById(elId);
                if (el != null) {
                    if (!el.classList.contains(newState)) {
                        el.classList.remove('up');
                        el.classList.remove('down');
                        el.classList.remove('mumble');
                        el.classList.remove('corrupt');
                        el.classList.remove('shit');
                        el.classList.remove('wait');
                        el.classList.remove('coffeebreak');
                        el.classList.add(newState);
                        // _animateElementOneTime(elId);
                        _animateElementServiceCell(elId);
                    }
                } else {
                    console.error(elId + '- not found');
                }
                var sCell = teamID + '-' + sService;
                // console.log(sCell);
                silentUpdate('att-' + sCell, newAttackFlags)
                // silentUpdate('def-' + sCell, newDefenceFlags)
                silentUpdate('pt_att-' + sCell, newAttackPoints.toFixed(2))
                silentUpdateWithoutAnimation('pt_def-' + sCell, newDefencePoints.toFixed(0))
                silentUpdate('sla-' + sCell, "SLA: " + newSLA + "%")
                silentUpdateWidthWithoutAnimation('sla-progress-' + sCell, newSLA + "%")
            }
        }

        // sort by places
        var elms2 = [];
        var elms = document.getElementsByClassName('tm');
        for(var i = 0; i < elms.length; i++){
            var el1 = elms[i];
            var place1 = parseInt(resp["scoreboard"][el1.id]['place'], 10);
            elms2.push({
                e: elms[i],
                p: place1
            });
        }
        elms2.sort(function(a, b) {
            return a.p - b.p;
        });
        for (var i = 0; i < elms2.length; i++) {
            var expected_top_value = (60 + (i+1)*50) + 'px'
            elms2[i].e.setAttribute("expected-top", expected_top_value);
            // if (elms2[i].e.style.top == '') {
            //     elms2[i].e.style.top = expected_top_value;
            // }
        }

        // open controls
        if (document.getElementById('scoreboard_content').style.display != 'block') {
            document.getElementById('scoreboard_content').style.display = 'block'
        }
        if (document.getElementById('loader_content').style.display != 'none') {
            document.getElementById('loader_content').style.display = 'none';
        }
    });
}

// animate switching
setInterval(function() {
    var elms = document.getElementsByClassName('tm');
    for (var i = 0; i < elms.length; i++) {
        var expected_top = parseInt(elms[i].getAttribute("expected-top"), 10);
        var current_top = parseInt(elms[i].style.top, 10);
        if (elms[i].style.top == '') {
            current_top = 0;
        }
        if (expected_top == current_top) {
            continue;
        }

        var diff = expected_top - current_top;
        if (Math.abs(diff) < 10) {
            current_top = expected_top;
        } else {
            current_top += Math.floor(diff / 10);
        }
        elms[i].style.top = current_top + 'px';
        // console.log();
    }
}, 40);

function formatGameTimings(periods) {
    // TODO beauty print periods
}

// init scoreboard
getAjax('/api/v1/game', function(err, resp){
    if (err) {
        console.error("Problem with game info ", err);
        return;
    }
    window.teams = resp.teams;
    document.getElementById('game_name').innerHTML = resp.game_name;

    // TODO beauty print periods
    if (resp.game_has_coffee_break) {
        document.getElementById('game_time_range').innerHTML =
            resp.game_start + ' - ' + resp.game_coffee_break_start + ' (coffee break) '
            + resp.game_coffee_break_end + ' - ' + resp.game_end;
    } else {
        document.getElementById('game_time_range').innerHTML = resp.game_start + ' - ' + resp.game_end;
    }

    // console.log(resp);

    // generate teams-services table
    var sContent = ""
        + "<div class='scoreboard' id='table_scoreboard'>"
        + "    <div class='hdrs'>"
        + "        <div class='place'>#</div>"
        + "        <div class='team-logo'></div>"
        + "        <div class='team'>Team</div>"
        + "        <div class='score'><div class='hdr-text'>points</div></div>";
    for (var i = 0; i < resp.services.length; i++) {
        var serviceId = resp.services[i].id;
        sContent += ''
        + '<div class="service"><b>' + resp.services[i].name + '</b><br>'
        + '  <div class="service-att-def">'
        + '      <div class="service-att-def-row">'
        + '          <div class="service-att-def-cell defence-flags" id="' + serviceId + '-all-flags-def">0</div>'
        + '          <div class="service-att-def-cell stollen-flags" id="' + serviceId + '-all-flags-att">0</div>'
        + '      </div>'
        + '      <div class="service-att-def-row">'
        + '          <div class="service-att-def-cell first-blood">'
        + '               <div class="tooltip">'
        + '                   <div class="first-blood-value" id="' + serviceId +  '-first-blood">-</div>'
        + '                   <div class="tooltiptext first-blood-info">'
        + '                     <div class="first-blood-info-value">First Blood!</div><br>'
        + '                     Service:'
        + '                     <div class="first-blood-info-value">' + escapeHtml(resp.services[i].name) + '</div><br>'
        + '                     Team Name: '
        + '                     <div class="first-blood-info-value" id="' + serviceId +  '-first-blood-teamname">-</div><br>'
        + '                     Time of First Blood from Start Game:'
        + '                     <div class="first-blood-info-value" id="' + serviceId +  '-first-blood-time">-</div><br>'
        + '                   </div>'
        + '               </div>'
        + '          </div>'
        + '          <div class="service-att-def-cell round-time">' + resp.services[i].round_time_in_sec + 's</div>'
        + '      </div>'
        + '  </div>'
        + "</div>";
    }
    sContent += ''
        + '        <div class="activity">Activity</div>'
        + '  </div>'
        + "  <div class='hdrs-time'>"
        + "    <div class='hdrs-time-fill' id='game_progress_time'></div>"
        + "    <div class='hdrs-time-game-current-time' id='game_current_time'>0</div>"
        + "  </div>";

    var sTeamListSelect = '';

    for (var iteam = 0; iteam < resp.teams.length; iteam++) {
        var sTeamId = resp.teams[iteam].id;
        var team_id = sTeamId;
        document.ctf01d_teams = resp.teams;
        sTeamListSelect += '<option value=' + sTeamId + '>' + sTeamId + '</option>';
        sContent += ""
            + "<div class='tm' id='" + sTeamId + "'>"
            + '  <div class="place" id="place-' + sTeamId + '" ></div>'
            + "  <div class='team-logo'><img class='team-logo' id='team-logo-" + sTeamId + "' logo_last_updated='0' src='" + resp.teams[iteam].logo + "'/></div>"
            + '  <div class="team tooltip">'
            + '    <div class="team-name">' + escapeHtml(resp.teams[iteam].name) + '</div>'
            + '    <span class="tooltiptext team-info">'
            + '     Team Name: ' + escapeHtml(resp.teams[iteam].name) + '<br>'
            + '     Team ID: <input id="' + team_id + '-copy" value="' + sTeamId + '"> <button onclick="copyToBuffer(\'' + team_id + '-copy\')">copy</button> <br>'
            + '     Team IP-Address: <input id="' + team_id + '-copy-ip" value="' + resp.teams[iteam].ip_address + '"> <button onclick="copyToBuffer(\'' + team_id + '-copy-ip\')">copy</button>'
            + '    </span>'
            + "  </div>"
            + '  <div class="score">'
            + '     <div class="points-sum" id="' + sTeamId + '-points">0</div>'
            + '     <div class="points-trend trend-down" id="' + sTeamId + '-points-trend">?</div>'
            + '  </div>';

        for (var i = 0; i < resp.services.length; i++) {
            var sServiceID = resp.services[i].id;
            sContent += ""
            + "<div class='service'>"
            + '  <div class="service-status down" id="status-' + sTeamId +  '-' + sServiceID + '"> '
            + '   <div class="service-att-def">'
            + '       <div class="service-att-def-row">'
            + '           <div class="service-att-def-cell first-column defence-points" id="pt_def-' + sTeamId +  '-' + sServiceID + '">0.0</div>'
            + '           <div class="service-att-def-cell attack-points">'
            + '              <div class="tooltip">'
            + '                 <div class="attack-points-value" id="pt_att-' + sTeamId +  '-' + sServiceID + '">0.0</div>'
            + '                 <span class="tooltiptext stollen-flags" id="att-' + sTeamId +  '-' + sServiceID + '">0</span>'
            + '              </div>'
            + '           </div>'
            + '       </div>'
            + '   </div>'
            + '  </div>'
            + '  <div class="service-sla-notify-container tooltip">'
            + '    <div class="service-sla-notify-progress" id="sla-progress-' + sTeamId +  '-' + sServiceID + '"></div>'
            + '    <span class="tooltiptext sla" id="sla-' + sTeamId +  '-' + sServiceID + '">SLA: 100%</span>'
            + '  </div>'
            + '</div>\n';
        }
        sContent += ""
            + '   <div class="activity">'
            + '      <div class="activity-value" id="tries-' + sTeamId +  '"></div>'
            + '      <div class="activity-icon" id="tries-icon-' + sTeamId +  '"></div>'
            + '   </div>'
            + "</div>";
    }
    sContent += "</div>";

    document.getElementById('scoreboard_content').innerHTML = sContent;
    document.getElementById('team_list').innerHTML = sTeamListSelect;

    updateScoreboard();
    setTimeout(updateTeamRequiredFields, 100);

    // start poling
    setInterval(function(){
        updateScoreboard()
    }, 3000);
});
