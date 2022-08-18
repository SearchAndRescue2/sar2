/**********************************************************************
*   This file is part of Search and Rescue II (SaR2).                 *
*                                                                     *
*   SaR2 is free software: you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License v.2 as       *
*   published by the Free Software Foundation.                        *
*                                                                     *
*   SaR2 is distributed in the hope that it will be useful, but       *
*   WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See          *
*   the GNU General Public License for more details.                  *
*                                                                     *
*   You should have received a copy of the GNU General Public License *
*   along with SaR2.  If not, see <http://www.gnu.org/licenses/>.     *
***********************************************************************/

// Returns true if string is a known tag
function isATag (string) {
    var tagList = new Array("NAME", "NAMES", "SEE", "SYNOPSIS", "DESCRIPTION", "ARGUMENTS", "CONTEXT", "EXAMPLE", "EXAMPLES", "-----");
    for (var i = 0; i < tagList.length; i++) {
        if (string === tagList[i]) return(true);
    }
    return(false);
}

// Transforms '##some_text' text into a hash or link to 'some_text'
function addHref(hrefInString) {
    var stringWord = hrefInString.split(" ");
    var hrefOutString = "";
    
    for (var i = 0; i < stringWord.length; i++) {
        if (stringWord[i].indexOf("##") >= 0) { // if this word must be transformed to a HTML link
            var paramName = "";
            for (var j = 2; j < stringWord[i].length; j++) paramName = paramName + stringWord[i][j];
            if (stringWord[i].indexOf('http') == 2) { // if stringWord[i] starts with "##http"
                hrefOutString = hrefOutString + "<a href='" + paramName + "' target='_blank'>" + paramName + "</a> "; // external link
            }
            else {
                hrefOutString = hrefOutString + "<a href='#" + paramName + "'>" + paramName + "</a> "; // within page link (hash)
            }
        }
        else { // not a link? Just copy it.
            hrefOutString = hrefOutString + stringWord[i] + " ";
        }
    }
    return(hrefOutString);
}

// Computes and sets blank 'footer' height: last section will always be shawn at top of page, even if it height if lower than page height.
function setBlankFooterHeight() {
    var menuEntries = document.getElementById('LeftMenu').getElementsByTagName('a');
    var lastDisplayedIndex = 0;
    for (var j = 0; j < menuEntries.length; j++) {
        if ( menuEntries[j].style.display === '' ) lastDisplayedIndex = j;
    }
    var lastMenuItemDestId = menuEntries[lastDisplayedIndex].getElementsByTagName('span')[0].innerHTML;
    var blankFooterHeight = document.getElementById('block0').clientHeight - document.getElementById(lastMenuItemDestId).clientHeight;
    document.getElementById('blank_footer').style.height = blankFooterHeight + "px";
}

// Show or hide parameters in vertical scroll menu, depending of checked "file type" checkboxes and parameter name filter input.
function showSelectedtypes() {
    var selectedTypesStorage = sessionStorage.getItem('sessionStoredSelectedTypes'); // read saved values

    if ( document.getElementById('misFile').checked == true )
        selectedTypesStorage = "mis";
    else
        selectedTypesStorage = "";
    
    if ( document.getElementById('3dFile').checked == true  )
        selectedTypesStorage = selectedTypesStorage + " mod";
    else
        selectedTypesStorage = selectedTypesStorage + "";
    
    if ( document.getElementById('scnFile').checked == true  )
        selectedTypesStorage = selectedTypesStorage + " scn";
    else
        selectedTypesStorage = selectedTypesStorage + "";

    sessionStorage.setItem('sessionStoredSelectedTypes', selectedTypesStorage); // store selectedTypesStorage for next page reload

    // Show selected parameters and hide other ones in vertical scroll menu
    leftMenuLinks = document.getElementsByClassName('leftMenuLink');
	var selectedTypes = selectedTypesStorage.split(" ");
    var input = document.getElementById("FilterInput");
    var filter = input.value.toUpperCase();
    for (var j=0;j<leftMenuLinks.length;j++) {
		txtValue = leftMenuLinks[j].innerText;
		if (!filter || (txtValue.toUpperCase().indexOf(filter) >= 0) ) showMe = true; // show parameter name only if filter is "" (null) 
		else showMe = false;
        if ( showMe && (leftMenuLinks[j].classList.contains(selectedTypes[0]) || leftMenuLinks[j].classList.contains(selectedTypes[1]) || leftMenuLinks[j].classList.contains(selectedTypes[2]) ) ) {
			leftMenuLinks[j].style.display='';
		}
		else {
			leftMenuLinks[j].style.display='none';
		}
    }
}

/* Clears filter when user clicked on dedicated button */
function filterClear() {
    document.getElementById("FilterInput").value = ""; // clear filter
    showSelectedtypes();
}

/* Shows only selected parameters names at page reload */
window.onload=function() {
    showSelectedtypes();
}

/* Function called when user has clicked on page. This is used to determine if page must be reloaded (see at end of hashHasChanged()).
 * Info: 'window.onpopstate' don't work because it is fired at each hash change, and not only when history is called.
 */
function userClicked() {
    sessionStorage.setItem('historyCalled', 'false'); // store historyCalled for next page reload
    sessionStorage.setItem('previousHash', window.location.hash); // store previousHash for next page reload
}

/* Function called when user has clicked on a link OR when user has cliqued on 'back' of 'forward' browser button (browser history call). */
function hashHasChanged() {
    var historyCalled = sessionStorage.getItem('historyCalled'); // read saved value
    var previousHash = sessionStorage.getItem('previousHash'); // read saved value
    /* If actual hash is same as previous hash, it's an history call). */
    if ( (window.location.hash === previousHash) || (historyCalled === 'true') ) {
        historyCalled = 'true';
        sessionStorage.setItem('historyCalled', historyCalled); // store historyCalled for next page reload
    }

    /* when user clicks on browser back button, check corresponding checkbox and show left menu links if they were hidden */
    if (false) { // It works, but is it a good idea? Let's inhibit it for now. 
        var destinationElemId = document.getElementById("left_menu_" + location.hash.substring(1));
        if (window.getComputedStyle(destinationElemId).display === "none") { // if menu link (i.e. link called by history) is hidden
            var classNames = destinationElemId.className.split(" ");
            for (var i = 0; i < classNames.length; i++) {
                if (classNames[i] === "mis") {
                    document.getElementById('misFile').checked = true;
                }
                if (classNames[i] === "mod") {
                    document.getElementById('3dFile').checked = true;
                }
                if (classNames[i] === "scn") {
                    document.getElementById('scnFile').checked = true;
                }
            }
        }
    }
    
    /* Page must be reloaded when user navigates through history, because if not browser will not show requested hash at top of page. */
    if ( historyCalled === 'true') {
        window.location.reload();
    }
}

/* Tool tip functions */
function getOffset(el) {
  var rect = el.getBoundingClientRect();
  return {
    left: rect.left + window.scrollX,
    top: rect.top + window.scrollY
  };
}
var toolTipActiveSpan = null;
function toolTipYPos(elem) {
    var y = getOffset(elem).top; // get '<a>' Y position
    var span = elem.getElementsByTagName('span')[0]; // get first '<span>' (i.e. tooltip) in current '<a>' element
    span.style.top = y + "px"; // set '<span>' tooltip Y position
    toolTipActiveSpan = span;

    var div = document.getElementById("LeftMenu");
    var scrollbarWidth = div.offsetWidth - div.clientWidth - parseFloat(getComputedStyle(div).borderLeftWidth) - parseFloat(getComputedStyle(div).borderRightWidth);
    
    if (div.offsetWidth - scrollbarWidth < span.offsetWidth) { // if parameter name is partially hidden (i.e. parameter name too long for left menu width)...
        span.style.visibility = "visible"; // show tooltip over scrollbar
        div.addEventListener("scroll", divScrolled); // to hide span when menu is scrolled
    }
}
function divScrolled() {
    toolTipActiveSpan.style.visibility = "hidden";
    document.getElementById("LeftMenu").removeEventListener("scroll", divScrolled);
}

function getBrowserName() { 
    if((navigator.userAgent.indexOf("Opera") || navigator.userAgent.indexOf('OPR')) != -1 ) {
        return 'Opera';
    } else if(navigator.userAgent.indexOf("Chrome") != -1 ) {
        return 'Chrome';
    } else if(navigator.userAgent.indexOf("Safari") != -1) {
        return 'Safari';
    } else if(navigator.userAgent.indexOf("Firefox") != -1 ){
        return 'Firefox';
    } else if((navigator.userAgent.indexOf("MSIE") != -1 ) || (!!document.documentMode == true )) {
        return 'Internet Explorer';
    } else {
        return 'Not sure!';
    }
}

/* Read cmdDescription variable (see sar2CmdDescription.js) and generate HTML code */
function main () {
    var subString = ""; // string to store a parsed ('__') part of cmdDescription string 
    var i = 0;
    var j = 0;
    var k = 0;
    var strOut = "";
    var strTest = "";
    var strSection = "";
    var tmpString = "";
    var hrefList = new Array();

    document.write("\n\n<!-- CONTENT AUTOMATICALLY GENERATED -->\n"); // add comment
    strOut = "<div id='block0' onclick=\"userClicked()\">\n"; // whole page

    strOut = strOut + "  <!-- LEFT MENU -->\n"; // add comment
    strOut = strOut + "  <div id='LeftMenuBlock'>\n";

    strOut = strOut + "    <div id='CheckBoxes'>\n";
    strOut = strOut + "      <input type=\"checkbox\" id='3dFile' onclick='showSelectedtypes()' checked>*.3d file\n"
    strOut = strOut + "      <input type=\"checkbox\" id='misFile' onclick='showSelectedtypes()' checked>*.mis file\n"
    strOut = strOut + "      <input type=\"checkbox\" id='scnFile' onclick='showSelectedtypes()' checked>*.scn file\n";
    strOut = strOut + "    </div> <!--div id='CheckBoxes'-->\n";
    
    strOut = strOut + "    <div id='Filter'>\n";
    strOut = strOut + "      <input id='FilterInput' type=\"text\" onkeyup='showSelectedtypes()' placeholder='Filter parameter names...'>\n";
    strOut = strOut + "      <button id='FilterDelete' type=\"button\" onclick='filterClear()'>&#10008;</button>\n";
    strOut = strOut + "    </div> <!--div id='Filter'-->\n";

    strOut = strOut + "    <nav id='LeftMenu'>\n";

    // left menu entries creation loop
    subString = cmdDescription.split("__");
    for (i = 1; i < subString.length; i++) { // subString[0] = ''
        if (subString[i] === "NAME" || subString[i] === "NAMES") { // then creates menu entries
            strOut = strOut + "      <a id='left_menu_" + subString[++i] + "' class='tbd' href='#" + subString[i] + "' onmouseover='toolTipYPos(this)'>" + subString[i]; // class = To Be Defined: will be set later in code
            strOut = strOut + "<span>" + subString[i] + "</span></a>\n";
            i++;
        }
    }

    strOut = strOut + "    </nav> <!--nav id='LeftMenu'-->\n"; // close 'nav' and add comment
    strOut = strOut + "  </div> <!--div id='LeftMenuBlock'-->\n"; // close 'div' and add comment

    strOut = strOut + "\n<!-- RIGHT CONTENT -->\n"; // add comment
    strOut = strOut + "  <div id='RightContentBlock'>\n";
    
    // Warning
    strOut = strOut + "    <div id='Warning'>\n";
    strOut = strOut + "      <span>This documentation is in development: there are some \"FIXME:\", maybe (I hope not!) some errors, and my english is not very good... All checks, tests and comments are welcome.</span>\n";
    strOut = strOut + "    </div> <!--div id='Warning'-->\n"; // close 'div' and add comment
    
    strOut = strOut + "    <div id='Content'>\n";
    document.write(strOut);
    // right content creation
    strTest = "";
    subString = cmdDescription.split("__");
    for (i = 1; i < subString.length; i++) { // subString[0] is empty
        j = 0;
        switch (subString[i]) {
            case "NAME":
            case "NAMES":
                strOut = "      <section id='" + subString[i+1] + "' "; // wait for class (see case "CONTEXT":)
                strSection = "";
                strSection = strSection + "        <h4>" + subString[i++] + "</h4>\n";
                while ( !isATag(subString[i]) ) {
                    if ( subString[i][0] === "-" &&  subString[i][1] === "-" && subString[i][2] === "-") {  // special case: if name starts with '---', it is not a parameter name.
                                                        // have a look at '---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;---'
                        strSection = strSection + "        <p><i>" + subString[i] + "</i></p>\n";
                    }
                    else strSection = strSection + "        <p>" + subString[i] + "</p>\n"; // normal case, it's a parameter name
                    i++;
                }
                i--;
                break;
            case "SEE":
                strSection = strSection + "        <p class='See' >See " + addHref(subString[++i]) + "</p>\n";
                break;
            case "DESCRIPTION":
                strSection = strSection + "        <h4>" + subString[i++] + "</h4>\n";
                while ( !isATag(subString[i]) ) strSection = strSection + "        <p>" + addHref(subString[i++]) + "</p>\n";
                i--;
                break;
            case "CONTEXT":
                strSection = strSection + "        <h4>" + subString[i++] + "</h4>\n";
                strTest = subString[i].split(" ");
                strOut = strOut + "class=\"";
                strSection = strSection + "        <p>";
                var moreThanOne = false;
                var knownContext = false;
                j = 0;
                while (j < strTest.length) {
                    if ( strTest[j] === "mis" ) {
                        knownContext = true;
                        if (moreThanOne) {
                            strOut = strOut + " ";
                            strSection = strSection + ", ";
                        }
                        strOut = strOut + "mis";
                        strSection = strSection + "mission file (*.mis) ";
                        moreThanOne = true;
                    }
                    if ( strTest[j] === "3d" ) {
                        knownContext = true;
                        if (moreThanOne) {
                            strOut = strOut + " ";
                            strSection = strSection + ", ";
                        }
                        strOut = strOut + "mod"; // 3d model
                        strSection = strSection + "model file (*.3d) ";
                        moreThanOne = true;
                    }
                    if ( strTest[j] === "scn" ) {
                        knownContext = true;
                        if (moreThanOne) {
                            strOut = strOut + " ";
                            strSection = strSection + ", ";
                        }
                        strOut = strOut + "scn";
                        strSection = strSection + "scenery file (*.scn) ";
                        moreThanOne = true;
                    }
                    if ( knownContext == false ) strSection = strSection + strTest[j]; // context is not one of mis, 3d or scn: just write it. Should never happen!
                    j++;
                }
                strOut = strOut + "\">\n"; // close <section> class
                strSection = strSection + "</p>\n";
                break;
            case "SYNOPSIS":
                strSection = strSection + "        <h4>" + subString[i++] + "</h4>\n";
                while ( !isATag(subString[i]) ) {
                    strSection = strSection + "        <p>";
                    strTest = subString[i].split(" ");
                    j = 0;
                    strSection = strSection + "<b>" + strTest[j++] + "</b>";
                    while (j < strTest.length) {
                        if ( strTest[j] === "[") {
                            strSection = strSection + "  [  ";
                        }
                        else if ( strTest[j] === "]") {
                            tmpString = tmpString + "  ]";
                        }
                        else {
                            strSection = strSection + "  &lt;" + strTest[j] + "&gt;";
                        }
                        j++;
                        strSection = strSection + tmpString;
                        tmpString = "";
                    }
                    strSection = strSection + "</p>\n";
                    i++;
                }
                i--;
                break;
            case "ARGUMENTS":
                strSection = strSection + "        <h4>" + subString[i++] + "</h4>\n        <table>\n          <tbody>\n";
                while ( !isATag(subString[i]) ) {
                    var argsList = subString[i].split(" ");
                    strSection = strSection + "            <tr>\n              <td>" + argsList[0] + "</td>\n              <td>";
                    j = 1;
                    while (j < argsList.length ) {
                        strSection = strSection + addHref(argsList[j++]);
                    }
                    strSection = strSection + "</td>\n            </tr>\n";
                    i++;
                }
                i--;
                strSection = strSection + "          </tbody>\n        </table>\n";
                break;
            case "EXAMPLE":
            case "EXAMPLES":
                strSection = strSection + "        <h4>" + subString[i++] + "</h4>\n";
                while ( !isATag(subString[i]) ) strSection = strSection + "        <p>" + addHref(subString[i++]) + "</p>\n";
                i--;
                break;
            case "-----":
                strSection = strSection + "        <br><hr>\n";
                strSection = strSection + "      </section>\n";
                strOut = strOut + strSection;
                strSection = "";
                document.write(strOut);
                strOut = "";
                break;
            default:
                strSection = strSection + "<h1>UNKNOWN ARGUMENT</h1>\n";
        }
    }
    document.write("      <section id='blank_footer'>\n      </section>\n"); // to have destination always be displayed at the top of the page ( see setBlankFooterHeight() )
    setBlankFooterHeight();

    document.write("    </div> <!--div id='Content'-->\n"); // close 'div' and add comment
    document.write("  </div> <!--div id='RightContentBlock'-->\n</div> <!--div id='block0'-->"); // close 'div's and add comments

    // it's time to replace "tbd" class in left menu <a> tags by destination class
    var elts= document.getElementsByClassName("tbd"); // list elements with "tbd" class, i.e. left menu elements. Note that section.length and elts.length are identical
    var eltsStatic=[];
    for( i = 0; i < elts.length; i++) eltsStatic.push(elts[i]); // copy node list to a static list because elts is a dynamic list and will change at each class name modification
    var section = document.getElementsByTagName("section"); // list <section> elements
    for( i = 0; i < eltsStatic.length; i++) {
        eltsStatic[i].setAttribute("class", section[i].className + " leftMenuLink tooltip"); // replace 'tbd' by <section>'s class name, and don't forget to add 'leftMenuLink' and 'tooltip' classes
    }

    // If there is no hash at page load, browser history will not be able to go back at top of page, thus, add it:
    subString = cmdDescription.split("__"); // subString[0] is empty, subString[1] = "NAME", and ...
    if (!location.hash) location.replace(location.href + "#" + subString[2]); // ... subString[2] contains the first hash name (should be "add_fire")

    // At first page load, 'sessionStoredSelectedTypes' value don't exist.
    if (!sessionStorage.getItem('sessionStoredSelectedTypes')) {
        sessionStorage.setItem('sessionStoredSelectedTypes', "mis mod scn"); // set 'sessionStoredSelectedTypes' value
        // Ooops
        if ( getBrowserName() === 'Chrome')
            alert("Your browser is Chrome.\nUnfortunately, you will not be able to use properly browser back/forward buttons (history) in this documentation.\nIt works with Firefox, but not with Chrome, and I don't know why...\nI'm really sorry about that :-(");
    }

} // main ()

/* Let's go */
main();
